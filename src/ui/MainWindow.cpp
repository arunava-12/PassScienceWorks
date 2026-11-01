
#define NOMINMAX
#include <windows.h>
#include <commdlg.h>
#include <windowsx.h>

#include "../core/geometry/STLParser.h"
#include "../core/geometry/TemperatureCSV.h"
#include "../core/thermal/ThermalSolver.h"
#include "../core/hardware/MockDevices.h"
#include "../core/performance/Benchmark.h"

#include <algorithm>
#include <atomic>
#include <memory>
#include <thread>
#include <filesystem>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace
{

    constexpr wchar_t WINDOW_CLASS[] = L"PassScienceWorkbenchWindow";

    constexpr int ID_GEOMETRY = 101;
    constexpr int ID_THERMAL = 102;
    constexpr int ID_PERFORMANCE = 103;
    constexpr int ID_HARDWARE = 104;
    constexpr int ID_LOAD_STL = 105;
    constexpr int ID_LOAD_CSV = 106;
    constexpr int ID_RUN_THERMAL = 107;
    constexpr int ID_CONNECT_DEVICES = 108;
    constexpr int ID_FIT_VIEW = 109;
    constexpr int ID_RESET_VIEW = 110;
    constexpr int ID_RUN_BENCHMARK = 111;
    constexpr int ID_DISCONNECT_DEVICES = 112;
    constexpr UINT WM_BENCHMARK_COMPLETE = WM_APP + 1;

    enum class View
    {
        Geometry,
        Thermal,
        Performance,
        Hardware
    };

    HWND g_window = nullptr;
    HWND g_status = nullptr;

    View g_view = View::Geometry;

    STLMesh g_mesh;
    TemperatureDataset g_dataset;

    bool g_hasMesh = false;
    bool g_hasDataset = false;
    bool g_hasThermalResult = false;

    ThermalResult g_thermalResult;

    double g_rotationX = 0.35;
    double g_rotationY = 0.45;
    double g_zoom = 1.0;

    bool g_dragging = false;
    bool g_panning = false;
    POINT g_lastMouse{};
    POINT g_mouseDown{};

    double g_panX = 0.0;
    double g_panY = 0.0;
    int g_selectedNodeId = -1;

    std::string g_message = "Ready. Load an STL or temperature CSV file.";

    hardware::MockMicrocontrollerAdapter g_mcu;
    hardware::MockFPGAAdapter g_fpga;
    hardware::MockJTAGAdapter g_jtag;

    BenchmarkReport g_benchmarkReport;
    bool g_hasBenchmarkReport = false;
    std::atomic<bool> g_benchmarkRunning{false};
    std::thread g_benchmarkThread;

    struct BenchmarkCompletion
    {
        bool succeeded = false;
        BenchmarkReport report;
        std::string error;
    };

    std::string toUtf8(const std::wstring &value)
    {
        if (value.empty())
        {
            return {};
        }

        const int size = WideCharToMultiByte(
            CP_UTF8, 0, value.c_str(), -1, nullptr, 0, nullptr, nullptr);

        if (size <= 1)
        {
            return {};
        }

        std::string result(static_cast<std::size_t>(size), '\0');

        WideCharToMultiByte(
            CP_UTF8, 0, value.c_str(), -1,
            result.data(), size, nullptr, nullptr);

        result.resize(static_cast<std::size_t>(size - 1));
        return result;
    }

    void setStatus(const std::string &message)
    {
        g_message = message;

        if (g_status)
        {
            const int size = MultiByteToWideChar(
                CP_UTF8, 0, message.c_str(), -1, nullptr, 0);

            std::wstring wide(static_cast<std::size_t>(size), L'\0');

            if (size > 0)
            {
                MultiByteToWideChar(
                    CP_UTF8, 0, message.c_str(), -1,
                    wide.data(), size);

                SetWindowTextW(g_status, wide.c_str());
            }
        }

        if (g_window)
        {
            InvalidateRect(g_window, nullptr, FALSE);
        }
    }

    void drawText(
        HDC dc,
        int x,
        int y,
        const std::wstring &value,
        COLORREF color = RGB(225, 230, 240),
        int fontSize = 16)
    {
        SetTextColor(dc, color);
        SetBkMode(dc, TRANSPARENT);

        HFONT font = CreateFontW(
            fontSize, 0, 0, 0, FW_NORMAL,
            FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH, L"Segoe UI");

        HFONT oldFont = reinterpret_cast<HFONT>(
            SelectObject(dc, font));

        TextOutW(
            dc, x, y,
            value.c_str(),
            static_cast<int>(value.size()));

        SelectObject(dc, oldFont);
        DeleteObject(font);
    }

    void drawTextA(
        HDC dc,
        int x,
        int y,
        const std::string &value,
        COLORREF color = RGB(225, 230, 240),
        int fontSize = 16)
    {
        const int size = MultiByteToWideChar(
            CP_UTF8, 0, value.c_str(), -1, nullptr, 0);

        std::wstring wide(static_cast<std::size_t>(size), L'\0');

        if (size > 0)
        {
            MultiByteToWideChar(
                CP_UTF8, 0, value.c_str(), -1,
                wide.data(), size);

            wide.resize(static_cast<std::size_t>(size - 1));
        }

        drawText(dc, x, y, wide, color, fontSize);
    }

    std::wstring viewName()
    {
        switch (g_view)
        {
        case View::Geometry:
            return L"Geometry Viewer";
        case View::Thermal:
            return L"Thermal Analysis";
        case View::Performance:
            return L"Performance Monitor";
        case View::Hardware:
            return L"Embedded Hardware";
        }

        return L"Workbench";
    }

    void createButton(
        HWND parent,
        const wchar_t *label,
        int id,
        int x,
        int y,
        int width = 145)
    {
        CreateWindowW(
            L"BUTTON",
            label,
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            x, y, width, 34,
            parent,
            reinterpret_cast<HMENU>(
                static_cast<INT_PTR>(id)),
            GetModuleHandleW(nullptr),
            nullptr);
    }

    bool chooseFile(
        HWND owner,
        const wchar_t *filter,
        std::wstring &selected)
    {
        wchar_t filename[MAX_PATH] = {};

        OPENFILENAMEW dialog{};
        dialog.lStructSize = sizeof(dialog);
        dialog.hwndOwner = owner;
        dialog.lpstrFilter = filter;
        dialog.lpstrFile = filename;
        dialog.nMaxFile = MAX_PATH;
        dialog.Flags =
            OFN_FILEMUSTEXIST |
            OFN_PATHMUSTEXIST |
            OFN_NOCHANGEDIR;

        if (!GetOpenFileNameW(&dialog))
        {
            return false;
        }

        selected = filename;
        return true;
    }

    struct ScreenPoint
    {
        int x;
        int y;
        double depth;
    };

    ScreenPoint projectPoint(
        const Vec3 &point,
        int centerX,
        int centerY,
        double scale)
    {
        const double cy = std::cos(g_rotationY);
        const double sy = std::sin(g_rotationY);
        const double cx = std::cos(g_rotationX);
        const double sx = std::sin(g_rotationX);

        const double rotatedX = point.x * cy - point.z * sy;
        const double rotatedZ = point.x * sy + point.z * cy;

        const double rotatedY = point.y * cx - rotatedZ * sx;
        const double depth = point.y * sx + rotatedZ * cx;

        const double perspective = 3.5 / (3.5 + depth);

        return {
            centerX + static_cast<int>(
                          g_panX + rotatedX * scale * g_zoom * perspective),
            centerY - static_cast<int>(
                          g_panY + rotatedY * scale * g_zoom * perspective),
            depth};
    }

    Vec3 normalizePoint(const Vec3 &point, const Vec3 &minimum,
                        const Vec3 &maximum)
    {
        const double centerX = (minimum.x + maximum.x) / 2.0;
        const double centerY = (minimum.y + maximum.y) / 2.0;
        const double centerZ = (minimum.z + maximum.z) / 2.0;

        const double rangeX = maximum.x - minimum.x;
        const double rangeY = maximum.y - minimum.y;
        const double rangeZ = maximum.z - minimum.z;

        const double largest = std::max({rangeX, rangeY, rangeZ, 1e-9});

        return {
            (point.x - centerX) * 2.0 / largest,
            (point.y - centerY) * 2.0 / largest,
            (point.z - centerZ) * 2.0 / largest};
    }


    COLORREF temperatureColor(double temperature, double minimum,
                              double maximum)
    {
        const double range = std::max(maximum - minimum, 1e-12);
        const double t = std::clamp(
            (temperature - minimum) / range, 0.0, 1.0);

        int red = 0;
        int green = 0;
        int blue = 0;

        if (t < 0.33)
        {
            const double local = t / 0.33;
            green = static_cast<int>(255.0 * local);
            blue = 255;
        }
        else if (t < 0.66)
        {
            const double local = (t - 0.33) / 0.33;
            green = 255;
            blue = static_cast<int>(255.0 * (1.0 - local));
        }
        else
        {
            const double local = (t - 0.66) / 0.34;
            red = static_cast<int>(255.0 * local);
            green = static_cast<int>(255.0 * (1.0 - local));
        }

        return RGB(red, green, blue);
    }

    void fitView()
    {
        g_rotationX = 0.35;
        g_rotationY = 0.45;
        g_zoom = 1.0;
        g_panX = 0.0;
        g_panY = 0.0;
        InvalidateRect(g_window, nullptr, FALSE);
    }

    void resetView()
    {
        fitView();
        g_selectedNodeId = -1;
        setStatus("Viewport reset.");
    }

    void inspectTemperatureNode(int mouseX, int mouseY)
    {
        if (!g_hasDataset || g_dataset.nodes.empty())
        {
            return;
        }

        Vec3 minimum{1e30, 1e30, 1e30};
        Vec3 maximum{-1e30, -1e30, -1e30};

        for (const auto& node : g_dataset.nodes)
        {
            minimum.x = std::min(minimum.x, node.x);
            minimum.y = std::min(minimum.y, node.y);
            minimum.z = std::min(minimum.z, node.z);
            maximum.x = std::max(maximum.x, node.x);
            maximum.y = std::max(maximum.y, node.y);
            maximum.z = std::max(maximum.z, node.z);
        }

        RECT client{};
        GetClientRect(g_window, &client);

        const int centerX = client.right / 2;
        const int centerY = (230 + client.bottom - 25) / 2;
        const double scale = std::min(
            client.right - 36, client.bottom - 255) * 0.30;

        double nearestDistance = 14.0 * 14.0;
        int nearestNode = -1;

        for (const auto& node : g_dataset.nodes)
        {
            const Vec3 normalized = normalizePoint(
                {node.x, node.y, node.z}, minimum, maximum);

            const ScreenPoint point = projectPoint(
                normalized, centerX, centerY, scale);

            const double dx = point.x - mouseX;
            const double dy = point.y - mouseY;
            const double distance = dx * dx + dy * dy;

            if (distance < nearestDistance)
            {
                nearestDistance = distance;
                nearestNode = node.nodeId;
            }
        }

        g_selectedNodeId = nearestNode;

        if (nearestNode >= 0)
        {
            setStatus("Selected temperature node " +
                      std::to_string(nearestNode) + ".");
        }
        else
        {
            setStatus("No temperature node selected.");
        }

        InvalidateRect(g_window, nullptr, FALSE);
    }

    void drawViewport(HDC dc, const RECT &area)
    {
        HBRUSH background = CreateSolidBrush(RGB(22, 27, 38));
        FillRect(dc, &area, background);
        DeleteObject(background);

        const int centerX = (area.left + area.right) / 2;
        const int centerY = (area.top + area.bottom) / 2;

        if (g_view == View::Geometry)
        {
            if (!g_hasMesh && !g_hasDataset)
            {
                drawText(
                    dc, centerX - 150, centerY,
                    L"Load an ASCII STL or temperature CSV file.",
                    RGB(160, 175, 195));
                return;
            }

            const double scale = std::min(
                area.right - area.left,
                area.bottom - area.top) * 0.30;

            if (g_hasMesh && !g_mesh.triangles.empty())
            {
                Vec3 minimum{1e30, 1e30, 1e30};
                Vec3 maximum{-1e30, -1e30, -1e30};

                for (const auto& triangle : g_mesh.triangles)
                {
                    for (const auto& vertex : triangle.vertices)
                    {
                        minimum.x = std::min(minimum.x, vertex.x);
                        minimum.y = std::min(minimum.y, vertex.y);
                        minimum.z = std::min(minimum.z, vertex.z);
                        maximum.x = std::max(maximum.x, vertex.x);
                        maximum.y = std::max(maximum.y, vertex.y);
                        maximum.z = std::max(maximum.z, vertex.z);
                    }
                }

                for (const auto& triangle : g_mesh.triangles)
                {
                    POINT points[3]{};

                    for (int i = 0; i < 3; ++i)
                    {
                        const Vec3 normalized = normalizePoint(
                            triangle.vertices[i], minimum, maximum);
                        const ScreenPoint projected = projectPoint(
                            normalized, centerX, centerY, scale);
                        points[i] = {projected.x, projected.y};
                    }

                    HPEN pen = CreatePen(
                        PS_SOLID, 1, RGB(75, 180, 235));
                    HPEN oldPen = reinterpret_cast<HPEN>(
                        SelectObject(dc, pen));
                    HBRUSH brush = CreateSolidBrush(
                        RGB(42, 105, 145));
                    HBRUSH oldBrush = reinterpret_cast<HBRUSH>(
                        SelectObject(dc, brush));

                    Polygon(dc, points, 3);

                    SelectObject(dc, oldPen);
                    SelectObject(dc, oldBrush);
                    DeleteObject(pen);
                    DeleteObject(brush);
                }

                drawTextA(dc, area.left + 20, area.top + 18,
                          "STL: " + g_mesh.name);
                drawTextA(dc, area.left + 20, area.top + 43,
                          "Triangles: " +
                          std::to_string(g_mesh.triangles.size()),
                          RGB(145, 190, 220), 13);
            }

            if (g_hasDataset && !g_dataset.nodes.empty())
            {
                Vec3 minimum{1e30, 1e30, 1e30};
                Vec3 maximum{-1e30, -1e30, -1e30};

                for (const auto& node : g_dataset.nodes)
                {
                    minimum.x = std::min(minimum.x, node.x);
                    minimum.y = std::min(minimum.y, node.y);
                    minimum.z = std::min(minimum.z, node.z);
                    maximum.x = std::max(maximum.x, node.x);
                    maximum.y = std::max(maximum.y, node.y);
                    maximum.z = std::max(maximum.z, node.z);
                }

                drawText(dc, area.left + 20,
                         g_hasMesh ? area.top + 72 : area.top + 18,
                         L"3D Temperature Point Cloud",
                         RGB(225, 230, 240), 17);

                for (const auto& node : g_dataset.nodes)
                {
                    const Vec3 normalized = normalizePoint(
                        {node.x, node.y, node.z}, minimum, maximum);
                    const ScreenPoint projected = projectPoint(
                        normalized, centerX, centerY, scale);
                    const COLORREF color = temperatureColor(
                        node.temperatureC,
                        g_dataset.minimumTemperature,
                        g_dataset.maximumTemperature);

                    HBRUSH brush = CreateSolidBrush(color);
                    HBRUSH oldBrush = reinterpret_cast<HBRUSH>(
                        SelectObject(dc, brush));
                    HPEN pen = CreatePen(
                        PS_SOLID, 1, RGB(235, 240, 250));
                    HPEN oldPen = reinterpret_cast<HPEN>(
                        SelectObject(dc, pen));

                    const int radius =
                        node.nodeId == g_selectedNodeId ? 9 : 5;

                    Ellipse(dc,
                            projected.x - radius,
                            projected.y - radius,
                            projected.x + radius,
                            projected.y + radius);

                    SelectObject(dc, oldBrush);
                    SelectObject(dc, oldPen);
                    DeleteObject(brush);
                    DeleteObject(pen);
                }

                const int legendX = area.right - 235;
                const int legendY = area.top + 45;
                const int legendWidth = 190;

                drawText(dc, legendX, legendY - 27,
                         L"Temperature (deg C)",
                         RGB(225, 230, 240), 13);

                for (int i = 0; i < legendWidth; ++i)
                {
                    const double fraction =
                        static_cast<double>(i) / (legendWidth - 1);
                    const double temperature =
                        g_dataset.minimumTemperature + fraction *
                        (g_dataset.maximumTemperature -
                         g_dataset.minimumTemperature);

                    HPEN pen = CreatePen(
                        PS_SOLID, 1,
                        temperatureColor(
                            temperature,
                            g_dataset.minimumTemperature,
                            g_dataset.maximumTemperature));
                    HPEN oldPen = reinterpret_cast<HPEN>(
                        SelectObject(dc, pen));

                    MoveToEx(dc, legendX + i, legendY, nullptr);
                    LineTo(dc, legendX + i, legendY + 14);

                    SelectObject(dc, oldPen);
                    DeleteObject(pen);
                }

                drawTextA(dc, legendX, legendY + 20,
                          std::to_string(
                              g_dataset.minimumTemperature) + " C",
                          RGB(160, 190, 220), 11);
                drawTextA(dc, legendX + 105, legendY + 20,
                          std::to_string(
                              g_dataset.maximumTemperature) + " C",
                          RGB(240, 150, 140), 11);

                drawTextA(dc, area.left + 20, area.bottom - 55,
                          "Temperature nodes: " +
                          std::to_string(g_dataset.nodes.size()),
                          RGB(170, 190, 210), 13);

                if (g_selectedNodeId >= 0)
                {
                    const auto selected = std::find_if(
                        g_dataset.nodes.begin(),
                        g_dataset.nodes.end(),
                        [](const TemperatureNode& node)
                        {
                            return node.nodeId == g_selectedNodeId;
                        });

                    if (selected != g_dataset.nodes.end())
                    {
                        std::ostringstream details;
                        details << std::fixed << std::setprecision(4)
                                << "Node " << selected->nodeId
                                << " | X: " << selected->x << " m"
                                << " | Y: " << selected->y << " m"
                                << " | Z: " << selected->z << " m"
                                << " | Temperature: "
                                << selected->temperatureC << " C";

                        drawTextA(dc, area.left + 20,
                                  area.bottom - 30, details.str(),
                                  RGB(255, 225, 150), 12);
                    }
                }
            }

            drawText(dc, area.left + 20, area.bottom - 10,
                     L"Left drag: rotate | Right drag: pan | Wheel: zoom",
                     RGB(165, 175, 195), 12);
            return;
        }

        if (g_view == View::Thermal)
        {
            if (g_hasThermalResult)
            {
                const int left = area.left + 55;
                const int right = area.right - 35;
                const int top = area.top + 65;
                const int bottom = area.bottom - 65;

                double minTemperature = *std::min_element(
                    g_thermalResult.temperatures.begin(),
                    g_thermalResult.temperatures.end());

                double maxTemperature = *std::max_element(
                    g_thermalResult.temperatures.begin(),
                    g_thermalResult.temperatures.end());

                drawText(
                    dc, left, area.top + 20,
                    L"Temperature distribution along the rod");
                drawTextA(dc, left, bottom + 48,
                          "X axis: Position (m) | Y axis: Temperature (deg C)",
                          RGB(165, 185, 205), 12);
                std::ostringstream rangeText;
                rangeText << std::fixed << std::setprecision(2)
                          << "Temperature range: " << minTemperature
                          << " to " << maxTemperature << " deg C";
                drawTextA(dc, left, area.top + 43, rangeText.str(),
                          RGB(165, 185, 205), 12);

                HPEN axisPen = CreatePen(
                    PS_SOLID, 1, RGB(160, 170, 185));

                HPEN oldPen = static_cast<HPEN>(
                    SelectObject(dc, axisPen));

                MoveToEx(dc, left, top, nullptr);
                LineTo(dc, left, bottom);
                LineTo(dc, right, bottom);

                const double range = std::max(
                    maxTemperature - minTemperature, 1e-9);

                HPEN graphPen = CreatePen(
                    PS_SOLID, 3, RGB(50, 190, 240));

                SelectObject(dc, graphPen);

                for (std::size_t i = 0;
                     i < g_thermalResult.temperatures.size(); ++i)
                {

                    const double xFraction =
                        static_cast<double>(i) /
                        static_cast<double>(
                            g_thermalResult.temperatures.size() - 1);

                    const double yFraction =
                        (g_thermalResult.temperatures[i] -
                         minTemperature) /
                        range;

                    const int x = left + static_cast<int>(
                                             xFraction * (right - left));

                    const int y = bottom - static_cast<int>(
                                               yFraction * (bottom - top));

                    if (i == 0)
                    {
                        MoveToEx(dc, x, y, nullptr);
                    }
                    else
                    {
                        LineTo(dc, x, y);
                    }
                }

                SelectObject(dc, oldPen);
                DeleteObject(axisPen);
                DeleteObject(graphPen);

                drawTextA(
                    dc, left, bottom + 20,
                    "Maximum midpoint error: " +
                        std::to_string(
                            g_thermalResult.maximumMidpointError) +
                        " C",
                    RGB(150, 210, 170), 13);

            }
            else
            {
                drawText(
                    dc, centerX - 125, centerY,
                    L"Run the thermal FEM solver.",
                    RGB(160, 175, 195));
            }

            return;
        }

        if (g_view == View::Performance)
        {
            drawText(
                dc, area.left + 35, area.top + 25,
                L"Serial vs parallel thermal benchmark",
                RGB(225, 230, 240), 18);

            if (!g_hasBenchmarkReport)
            {
                drawText(
                    dc, area.left + 35, area.top + 75,
                    L"Select Run Benchmark to generate fresh results.",
                    RGB(180, 195, 215), 14);
                drawText(
                    dc, area.left + 35, area.top + 105,
                    L"Workload: deterministic thermal cases; 5 measured repetitions.",
                    RGB(150, 170, 195), 12);
                return;
            }

            std::ostringstream hardwareLine;
            hardwareLine << "Logical CPUs: "
                         << g_benchmarkReport.logicalCpuCount
                         << " | Cases: " << g_benchmarkReport.numberOfCases
                         << " | Seed: " << g_benchmarkReport.seed
                         << " | Repetitions: " << g_benchmarkReport.repetitions;

            drawTextA(
                dc, area.left + 35, area.top + 65,
                hardwareLine.str(), RGB(165, 190, 215), 13);

            const int tableX = area.left + 35;
            int y = area.top + 110;

            drawText(
                dc, tableX, y,
                L"Workers       Median (ms)       Speedup       Efficiency       Max diff (C)",
                RGB(240, 205, 135), 13);
            y += 32;

            for (const auto& result : g_benchmarkReport.results)
            {
                std::ostringstream row;
                row << std::fixed << std::setprecision(4)
                    << std::left << std::setw(14) << result.workers
                    << std::setw(19) << result.medianMilliseconds
                    << std::setw(15) << result.speedup
                    << std::setw(17) << result.efficiency
                    << result.maximumTemperatureDifference;

                drawTextA(
                    dc, tableX, y, row.str(),
                    RGB(195, 215, 230), 13);
                y += 30;
            }

            drawText(
                dc, tableX, y + 20,
                L"Numerical consistency tolerance: 1e-8 deg C",
                RGB(145, 205, 165), 12);

            drawText(
                dc, tableX, y + 43,
                L"Timing excludes compilation, workload generation and CSV export.",
                RGB(160, 175, 195), 12);
            return;
        }

        if (g_view == View::Hardware)
        {
            drawText(
                dc, area.left + 35, area.top + 25,
                L"Embedded hardware devices",
                RGB(225, 230, 240), 18);

            struct DeviceDisplay
            {
                hardware::DeviceAdapter* adapter;
                const wchar_t* category;
            };

            const DeviceDisplay devices[] = {
                {&g_mcu, L"MICROCONTROLLER"},
                {&g_fpga, L"FPGA"},
                {&g_jtag, L"JTAG DEBUG ADAPTER"}
            };

            int y = area.top + 70;

            for (const auto& item : devices)
            {
                const auto info = item.adapter->discover();

                drawText(
                    dc, area.left + 40, y,
                    item.category, RGB(240, 205, 135), 12);
                y += 23;

                drawTextA(
                    dc, area.left + 40, y,
                    info.name + " | ID: " + info.id,
                    RGB(220, 230, 240), 14);
                y += 23;

                drawTextA(
                    dc, area.left + 40, y,
                    "Interface: " + info.connection +
                    " | State: " +
                    (item.adapter->isConnected() ? "CONNECTED" : "DISCONNECTED") +
                    " | SIMULATED",
                    item.adapter->isConnected()
                        ? RGB(120, 210, 155)
                        : RGB(185, 190, 205),
                    12);
                y += 23;

                if (item.adapter->isConnected())
                {
                    const auto telemetry = item.adapter->readTelemetry();
                    std::ostringstream reading;
                    reading << std::fixed << std::setprecision(2)
                            << telemetry.message << ": "
                            << telemetry.value << " "
                            << telemetry.unit << " [SIMULATED]";

                    drawTextA(
                        dc, area.left + 55, y, reading.str(),
                        RGB(155, 200, 235), 12);
                    y += 22;
                }
                else
                {
                    drawText(
                        dc, area.left + 55, y,
                        L"Connect devices to display simulated telemetry.",
                        RGB(145, 160, 180), 11);
                    y += 22;
                }

                const auto capabilities = item.adapter->capabilities();
                std::ostringstream capabilityLine;
                capabilityLine << "Capabilities: ";

                for (std::size_t i = 0; i < capabilities.size(); ++i)
                {
                    if (i > 0)
                    {
                        capabilityLine << " | ";
                    }
                    capabilityLine << capabilities[i];
                }

                drawTextA(
                    dc, area.left + 55, y,
                    capabilityLine.str(), RGB(170, 180, 200), 11);
                y += 43;
            }

            drawText(
                dc, area.left + 35, area.bottom - 30,
                L"Simulation only: no physical hardware is connected.",
                RGB(240, 190, 120), 13);
        }
    }

    void loadSTL()
    {
        const wchar_t filter[] =
            L"ASCII STL Files (*.stl)\0*.stl\0All Files (*.*)\0*.*\0";

        std::wstring filename;

        if (!chooseFile(g_window, filter, filename))
        {
            return;
        }

        try
        {
            g_mesh = STLParser::parseASCII(toUtf8(filename));
            g_hasMesh = true;

            g_view = View::Geometry;

            setStatus(
                "Loaded STL: " +
                std::to_string(g_mesh.triangles.size()) +
                " triangles.");
        }
        catch (const std::exception &error)
        {
            setStatus(
                "STL import failed: " + std::string(error.what()));

            MessageBoxA(
                g_window,
                error.what(),
                "STL Import Error",
                MB_OK | MB_ICONERROR);
        }
    }

    void loadTemperatureCSV()
    {
        const wchar_t filter[] =
            L"CSV Files (*.csv)\0*.csv\0All Files (*.*)\0*.*\0";

        std::wstring filename;

        if (!chooseFile(g_window, filter, filename))
        {
            return;
        }

        try
        {
            g_dataset = TemperatureCSV::parse(toUtf8(filename));
            g_hasDataset = true;

            g_view = View::Geometry;

            setStatus(
                "Loaded temperature dataset: " +
                std::to_string(g_dataset.nodes.size()) +
                " nodes.");
        }
        catch (const std::exception &error)
        {
            setStatus(
                "CSV import failed: " + std::string(error.what()));

            MessageBoxA(
                g_window,
                error.what(),
                "CSV Import Error",
                MB_OK | MB_ICONERROR);
        }
    }

    void runThermal()
    {
        try
        {
            ThermalParameters parameters;
            parameters.elements = 80;

            g_thermalResult = ThermalSolver::solve(parameters);
            g_hasThermalResult = true;

            ThermalSolver::exportCSV(
                g_thermalResult,
                "results/csv/ui_thermal_result.csv");

            g_view = View::Thermal;

            setStatus(
                "Thermal analysis completed. Result exported.");
        }
        catch (const std::exception &error)
        {
            setStatus(
                "Thermal solver failed: " + std::string(error.what()));

            MessageBoxA(
                g_window,
                error.what(),
                "Thermal Analysis Error",
                MB_OK | MB_ICONERROR);
        }
    }

    void runBenchmarkFromUI()
    {
        if (g_benchmarkRunning.load())
        {
            setStatus("Benchmark is already running in the background.");
            return;
        }

        if (g_benchmarkThread.joinable())
        {
            g_benchmarkThread.join();
        }

        BenchmarkConfig config;
        config.seed = 20261101;
        config.numberOfCases = 128;
        config.minimumElements = 100;
        config.maximumElements = 150;
        config.repetitions = 5;
        config.warmupRuns = 1;
        config.maximumWorkers = 4;

        g_benchmarkRunning.store(true);
        setStatus("Benchmark running in background. The workbench remains responsive...");

        g_benchmarkThread = std::thread([config]()
        {
            auto completion = std::make_unique<BenchmarkCompletion>();

            try
            {
                completion->report = Benchmark::run(config);
                std::filesystem::create_directories("results/csv");
                Benchmark::exportCSV(
                    completion->report,
                    "results/csv/performance_benchmark.csv");
                completion->succeeded = true;
            }
            catch (const std::exception& error)
            {
                completion->error = error.what();
            }
            catch (...)
            {
                completion->error = "Unknown error while running benchmark.";
            }

            BenchmarkCompletion* result = completion.release();
            if (!PostMessageW(
                    g_window,
                    WM_BENCHMARK_COMPLETE,
                    0,
                    reinterpret_cast<LPARAM>(result)))
            {
                delete result;
            }
        });
    }

    void disconnectHardware()
    {
        const auto mcu = g_mcu.disconnect();
        const auto fpga = g_fpga.disconnect();
        const auto jtag = g_jtag.disconnect();

        g_view = View::Hardware;

        setStatus(
            "Devices disconnected. MCU: " + mcu.message +
            " FPGA: " + fpga.message +
            " JTAG: " + jtag.message);
    }

    void connectHardware()
    {
        const auto mcu = g_mcu.connect(1000);
        const auto fpga = g_fpga.connect(1000);
        const auto jtag = g_jtag.connect(1000);

        g_view = View::Hardware;

        std::ostringstream message;

        message << "MCU: " << mcu.message
                << " | FPGA: " << fpga.message
                << " | JTAG: " << jtag.message;

        setStatus(message.str());
    }

    void createControls(HWND window)
    {
        createButton(window, L"Geometry", ID_GEOMETRY, 18, 18);
        createButton(window, L"Thermal", ID_THERMAL, 170, 18);
        createButton(window, L"Performance", ID_PERFORMANCE, 322, 18);
        createButton(window, L"Hardware", ID_HARDWARE, 474, 18);

        createButton(window, L"Load STL", ID_LOAD_STL, 18, 70);
        createButton(window, L"Load Temperature CSV", ID_LOAD_CSV, 170, 70, 190);
        createButton(window, L"Run Thermal FEM", ID_RUN_THERMAL, 368, 70, 165);
        createButton(window, L"Connect Devices", ID_CONNECT_DEVICES, 541, 70, 165);
        createButton(window, L"Fit View", ID_FIT_VIEW, 18, 150);
        createButton(window, L"Reset View", ID_RESET_VIEW, 170, 150);
        createButton(window, L"Run Benchmark", ID_RUN_BENCHMARK, 322, 150, 165);
        createButton(window, L"Disconnect Devices", ID_DISCONNECT_DEVICES, 494, 150, 175);

        g_status = CreateWindowW(
            L"STATIC",
            L"Ready.",
            WS_CHILD | WS_VISIBLE,
            18, 112, 900, 28,
            window,
            nullptr,
            GetModuleHandleW(nullptr),
            nullptr);
    }

    LRESULT CALLBACK windowProcedure(
        HWND window,
        UINT message,
        WPARAM wParam,
        LPARAM lParam)
    {
        switch (message)
        {
        case WM_CREATE:
            createControls(window);
            return 0;

        case WM_BENCHMARK_COMPLETE:
        {
            std::unique_ptr<BenchmarkCompletion> completion(
                reinterpret_cast<BenchmarkCompletion*>(lParam));
            g_benchmarkRunning.store(false);

            if (g_benchmarkThread.joinable())
            {
                g_benchmarkThread.join();
            }

            if (completion && completion->succeeded)
            {
                g_benchmarkReport = std::move(completion->report);
                g_hasBenchmarkReport = true;
                g_view = View::Performance;
                setStatus("Benchmark completed and exported to results/csv/performance_benchmark.csv.");
            }
            else
            {
                const std::string error = completion
                    ? completion->error
                    : "Benchmark completion message was empty.";
                setStatus("Benchmark failed: " + error);
                MessageBoxA(window, error.c_str(), "Benchmark Error",
                            MB_OK | MB_ICONERROR);
            }

            InvalidateRect(window, nullptr, FALSE);
            return 0;
        }

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
            case ID_GEOMETRY:
                g_view = View::Geometry;
                setStatus("Geometry view selected.");
                break;

            case ID_THERMAL:
                g_view = View::Thermal;
                setStatus("Thermal analysis view selected.");
                break;

            case ID_PERFORMANCE:
                g_view = View::Performance;
                setStatus("Performance view selected.");
                break;

            case ID_HARDWARE:
                g_view = View::Hardware;
                setStatus("Hardware view selected. Simulated devices only.");
                break;

            case ID_LOAD_STL:
                loadSTL();
                break;

            case ID_LOAD_CSV:
                loadTemperatureCSV();
                break;

            case ID_RUN_THERMAL:
                runThermal();
                break;

            case ID_CONNECT_DEVICES:
                connectHardware();
                break;

            case ID_FIT_VIEW:
                fitView();
                setStatus("Viewport fitted to default view.");
                break;

            case ID_RESET_VIEW:
                resetView();
                break;

            case ID_RUN_BENCHMARK:
                runBenchmarkFromUI();
                break;

            case ID_DISCONNECT_DEVICES:
                disconnectHardware();
                break;
            }

            InvalidateRect(window, nullptr, FALSE);
            return 0;

        case WM_PAINT:
        {
            PAINTSTRUCT paint{};
            HDC dc = BeginPaint(window, &paint);

            RECT client{};
            GetClientRect(window, &client);

            HBRUSH background = CreateSolidBrush(
                RGB(15, 19, 28));

            FillRect(dc, &client, background);
            DeleteObject(background);

            drawText(
                dc, 18, 190,
                viewName(),
                RGB(235, 240, 250), 21);

            RECT viewport{
                18,
                230,
                client.right - 18,
                client.bottom - 25};

            drawViewport(dc, viewport);

            EndPaint(window, &paint);
            return 0;
        }

        case WM_LBUTTONDOWN:
            g_dragging = true;
            g_lastMouse.x = GET_X_LPARAM(lParam);
            g_lastMouse.y = GET_Y_LPARAM(lParam);
            g_mouseDown = g_lastMouse;
            SetCapture(window);
            return 0;

        case WM_LBUTTONUP:
        {
            const int mouseX = GET_X_LPARAM(lParam);
            const int mouseY = GET_Y_LPARAM(lParam);
            const int dx = mouseX - g_mouseDown.x;
            const int dy = mouseY - g_mouseDown.y;

            g_dragging = false;
            ReleaseCapture();

            if (std::abs(dx) < 3 && std::abs(dy) < 3 &&
                g_view == View::Geometry)
            {
                inspectTemperatureNode(mouseX, mouseY);
            }
            return 0;
        }

        case WM_RBUTTONDOWN:
            g_panning = true;
            g_lastMouse.x = GET_X_LPARAM(lParam);
            g_lastMouse.y = GET_Y_LPARAM(lParam);
            SetCapture(window);
            return 0;

        case WM_RBUTTONUP:
            g_panning = false;
            ReleaseCapture();
            return 0;

        case WM_MOUSEMOVE:
            if (g_dragging && g_view == View::Geometry)
            {
                const int x = GET_X_LPARAM(lParam);
                const int y = GET_Y_LPARAM(lParam);

                g_rotationY += (x - g_lastMouse.x) * 0.01;
                g_rotationX += (y - g_lastMouse.y) * 0.01;

                g_lastMouse.x = x;
                g_lastMouse.y = y;

                InvalidateRect(window, nullptr, FALSE);
            }
            else if (g_panning && g_view == View::Geometry)
            {
                const int x = GET_X_LPARAM(lParam);
                const int y = GET_Y_LPARAM(lParam);

                g_panX += x - g_lastMouse.x;
                g_panY += y - g_lastMouse.y;

                g_lastMouse.x = x;
                g_lastMouse.y = y;

                InvalidateRect(window, nullptr, FALSE);
            }
            return 0;

        case WM_MOUSEWHEEL:
            if (g_view == View::Geometry)
            {
                const short delta = GET_WHEEL_DELTA_WPARAM(wParam);

                g_zoom *= delta > 0 ? 1.1 : 0.9;
                g_zoom = std::clamp(g_zoom, 0.1, 10.0);

                InvalidateRect(window, nullptr, FALSE);
            }
            return 0;

        case WM_DESTROY:
            if (g_benchmarkThread.joinable())
            {
                g_benchmarkThread.join();
            }
            g_mcu.disconnect();
            g_fpga.disconnect();
            g_jtag.disconnect();

            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProcW(
            window, message, wParam, lParam);
    }

}

int WINAPI WinMain(
    HINSTANCE instance,
    HINSTANCE,
    LPSTR,
    int showCommand)
{
    WNDCLASSW windowClass{};

    windowClass.lpfnWndProc = windowProcedure;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = WINDOW_CLASS;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.hbrBackground =
        reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));

    if (!RegisterClassW(&windowClass))
    {
        MessageBoxW(
            nullptr,
            L"Could not register the application window.",
            L"Startup Error",
            MB_OK | MB_ICONERROR);

        return 1;
    }

    g_window = CreateWindowExW(
        0,
        WINDOW_CLASS,
        L"PASS SCIENCE WORKS | Engineering Workbench",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1150,
        760,
        nullptr,
        nullptr,
        instance,
        nullptr);

    if (!g_window)
    {
        MessageBoxW(
            nullptr,
            L"Could not create the application window.",
            L"Startup Error",
            MB_OK | MB_ICONERROR);

        return 1;
    }

    ShowWindow(g_window, showCommand);
    UpdateWindow(g_window);

    MSG message{};

    while (GetMessageW(&message, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return static_cast<int>(message.wParam);
}
