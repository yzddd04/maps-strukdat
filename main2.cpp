#include <iostream>
#include <graphics.h>
#include <vector>
#include <queue>
#include <limits>
#include <cmath>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <conio.h>
#include <algorithm>  // Added for sort and reverse functions

using namespace std;

struct Point {
    int x, y;
    string name;
    Point(int x = 0, int y = 0, string name = "") : x(x), y(y), name(name) {}
};

struct Edge {
    int to;
    double weight;
    Edge(int to, double weight) : to(to), weight(weight) {}
};

class GoogleMapsSimulator {
private:
    vector<Point> locations;
    vector<vector<Edge>> graph;
    int numLocations;
    
    // Skala dan offset untuk menampilkan koordinat di layar
    double SCALE_X = 0.08;
    double SCALE_Y = 0.08;
    int OFFSET_X = 50;
    int OFFSET_Y = 50;
    int WINDOW_WIDTH = 1920;
    int WINDOW_HEIGHT = 1080;
    const double UNIT_TO_KM = 1.0 / 1000.0;
    
    // Menghitung jarak Euclidean antara dua titik
    double calculateDistance(const Point& a, const Point& b) {
        return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
    }
    
    // Konversi koordinat ke layar
    int toScreenX(int worldX) {
        return (int)(worldX * SCALE_X) + OFFSET_X;
    }
    
    int toScreenY(int worldY) {
        return WINDOW_HEIGHT - ((int)(worldY * SCALE_Y) + OFFSET_Y);
    }
    
public:
    GoogleMapsSimulator() {
        // Inisialisasi 20 lokasi sesuai koordinat yang diberikan
        locations = {
            Point(2070, 2995, "Pesantren Islam Al Irsyad"),
            Point(1810, 3400, "Penginapan Ummu Yasmin"),
            Point(575, 2525, "Raff Kos"),
            Point(370, 2180, "GCC Makmur Indonesia Project"),
            Point(1625, 1755, "Pesantren Islam Al Irsyad Putri"),
            Point(3095, 1720, "Penginapan Walisantri AMMA"),
            Point(2515, 685, "Lapangan Desa Butuh"),
            Point(3915, 390, "Geral Samsat Tengaran"),
            Point(3860, 730, "SPBU PERTAMINA Butuh"),
            Point(4470, 575, "Joglo Kebon Ndhelik"),
            Point(5780, 1285, "Lapangan Karang Duren"),
            Point(6650, 1775, "Kezia Grosir Ikan Hias Murah"),
            Point(4505, 1665, "Ponpes Nurul Islam Tengaran"),
            Point(4620, 2170, "PT Japfa Comfeed Indonesia"),
            Point(5080, 2450, "Amelia House"),
            Point(5790, 3325, "Musholla Arrahman"),
            Point(5400, 3565, "Iguana Kos"),
            Point(3630, 3555, "Rocket Chicken Tengaran"),
            Point(3825, 2385, "SPBU PERTAMINA Klero"),
            Point(2910, 2745, "Masjid Sabilul Khairat"),
        };
        
        numLocations = locations.size();
        graph.resize(numLocations);
        
        // Hitung bounding box lokasi
        int minX = locations[0].x, maxX = locations[0].x;
        int minY = locations[0].y, maxY = locations[0].y;
        for (const auto& p : locations) {
            if (p.x < minX) minX = p.x;
            if (p.x > maxX) maxX = p.x;
            if (p.y < minY) minY = p.y;
            if (p.y > maxY) maxY = p.y;
        }
        int mapWidth = maxX - minX;
        int mapHeight = maxY - minY;
        // Window size sudah fix 1000x700
        SCALE_X = (WINDOW_WIDTH - 200.0) / mapWidth;
        SCALE_Y = (WINDOW_HEIGHT - 200.0) / mapHeight;
        // Offset agar peta di tengah
        OFFSET_X = (WINDOW_WIDTH - (int)(mapWidth * SCALE_X)) / 2 - (int)(minX * SCALE_X);
        OFFSET_Y = (WINDOW_HEIGHT - (int)(mapHeight * SCALE_Y)) / 2 - (int)(minY * SCALE_Y);
        
        // Membuat graph dengan menghubungkan setiap lokasi ke lokasi terdekat
        buildGraph();
    }
    
    void buildGraph() {
        // Membuat graph realistis - hanya menghubungkan dengan titik terdekat
        // Tidak menggunakan complete graph, tapi berdasarkan jarak terdekat
        
        const int MAX_CONNECTIONS = 5; // Maksimal 5 koneksi per lokasi
        const double MAX_DISTANCE = 2000; // Maksimal jarak koneksi langsung
        
        for (int i = 0; i < numLocations; i++) {
            // Buat vector untuk menyimpan jarak ke semua titik lain
            vector<pair<double, int>> distances;
            
            for (int j = 0; j < numLocations; j++) {
                if (i != j) {
                    double distance = calculateDistance(locations[i], locations[j]);
                    distances.push_back({distance, j});
                }
            }
            
            // Urutkan berdasarkan jarak terdekat
            sort(distances.begin(), distances.end());
            
            // Ambil maksimal MAX_CONNECTIONS titik terdekat
            int connectionCount = 0;
            for (auto& dist : distances) {
                if (connectionCount >= MAX_CONNECTIONS) break;
                if (dist.first > MAX_DISTANCE) break; // Jangan terlalu jauh
                
                graph[i].push_back(Edge(dist.second, dist.first));
                connectionCount++;
            }
            
            // Pastikan setiap lokasi minimal terhubung ke 2 titik terdekat
            if (connectionCount < 2 && distances.size() >= 2) {
                for (int k = connectionCount; k < min(2, (int)distances.size()); k++) {
                    graph[i].push_back(Edge(distances[k].second, distances[k].first));
                }
            }
        }
        
        // Tambahkan koneksi bidirectional untuk memastikan graf terhubung
        ensureBidirectionalConnections();
    }
    
    void ensureBidirectionalConnections() {
        // Pastikan jika A terhubung ke B, maka B juga terhubung ke A
        vector<vector<bool>> connected(numLocations, vector<bool>(numLocations, false));
        
        // Tandai koneksi yang sudah ada
        for (int i = 0; i < numLocations; i++) {
            for (const Edge& edge : graph[i]) {
                connected[i][edge.to] = true;
            }
        }
        
        // Tambahkan koneksi balik jika belum ada
        for (int i = 0; i < numLocations; i++) {
            for (int j = 0; j < numLocations; j++) {
                if (connected[i][j] && !connected[j][i]) {
                    double distance = calculateDistance(locations[i], locations[j]);
                    graph[j].push_back(Edge(i, distance));
                    connected[j][i] = true;
                }
            }
        }
    }
    
    void initializeGraphics() {
        // Perbesar ukuran window BGI
        initwindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Google Maps Simulator");
        setbkcolor(BLUE);
        cleardevice();
    }
    
    void drawMap() {
        cleardevice();
        setcolor(WHITE);
        
        // Judul
        settextstyle(BOLD_FONT, HORIZ_DIR, 3); // Perbesar font judul
        outtextxy(10, 10, const_cast<char*>("Google Maps Simulator v0.01  |  Peta Lokasi SMP"));
        
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
        
        // Gambar grid
        setcolor(DARKGRAY);
        for (int i = 0; i < getmaxx(); i += 50) {
            line(i, 0, i, getmaxy());
        }
        for (int i = 0; i < getmaxy(); i += 50) {
            line(0, i, getmaxx(), i);
        }
        
        // Gambar koneksi jalan (edges) terlebih dahulu
        setcolor(LIGHTGRAY);
        setlinestyle(SOLID_LINE, 0, 1);
        for (int i = 0; i < numLocations; i++) {
            int x1 = toScreenX(locations[i].x);
            int y1 = toScreenY(locations[i].y);
            
            for (const Edge& edge : graph[i]) {
                int x2 = toScreenX(locations[edge.to].x);
                int y2 = toScreenY(locations[edge.to].y);
                line(x1, y1, x2, y2);
                // Tampilkan label jarak di tengah edge, hanya sekali per edge
                if (i < edge.to) {
                    settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
                    setcolor(WHITE);
                    int midX = (x1 + x2) / 2;
                    int midY = (y1 + y2) / 2;
                    char label[32];
                    sprintf(label, "%.2f", edge.weight * UNIT_TO_KM);
                    outtextxy(midX, midY, label);
                }
            }
        }
        
        // Gambar lokasi di atas garis
        for (int i = 0; i < numLocations; i++) {
            int screenX = toScreenX(locations[i].x);
            int screenY = toScreenY(locations[i].y);
            
            // Gambar titik lokasi
            setcolor(RED);
            setfillstyle(SOLID_FILL, RED);
            fillellipse(screenX, screenY, 3, 3);
            
            // Label nomor dan nama lokasi
            setcolor(WHITE);
            settextstyle(DEFAULT_FONT, HORIZ_DIR, 2); // Perbesar font angka
            char label[256];
            sprintf(label, "%d. %s", i + 1, locations[i].name.c_str());
            outtextxy(screenX + 10, screenY - 10, label);
            settextstyle(DEFAULT_FONT, HORIZ_DIR, 1); // Kembalikan font ke default
        }
        
        // Legend (pindahkan ke kiri bawah, font lebih besar)
        int legendY = getmaxy() - 140;
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 2); // Perbesar font legenda
        setcolor(WHITE);
        outtextxy(20, legendY, const_cast<char*>("Legenda:"));
        setcolor(RED);
        fillellipse(20, legendY + 25, 6, 6);
        setcolor(WHITE);
        outtextxy(35, legendY + 15, const_cast<char*>("Lokasi Penting"));
        setcolor(LIGHTGRAY);
        line(15, legendY + 50, 35, legendY + 50);
        setcolor(WHITE);
        outtextxy(35, legendY + 40, const_cast<char*>("Jalan Penghubung"));
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
        outtextxy(35, legendY + 70, const_cast<char*>("Jarak pada edge dalam satuan KM"));
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
        outtextxy(35, getmaxy() - 25, const_cast<char*>("Tekan ESC untuk kembali ke menu"));
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
    }
    
    void drawPath(const vector<int>& path) {
        if (path.size() < 2) return;
        
        setcolor(YELLOW);
        setlinestyle(SOLID_LINE, 0, 3);
        
        for (int i = 0; i < path.size() - 1; i++) {
            int x1 = toScreenX(locations[path[i]].x);
            int y1 = toScreenY(locations[path[i]].y);
            int x2 = toScreenX(locations[path[i + 1]].x);
            int y2 = toScreenY(locations[path[i + 1]].y);
            
            line(x1, y1, x2, y2);
        }
        
        // Highlight start and end points
        setcolor(GREEN);
        setfillstyle(SOLID_FILL, GREEN);
        int startX = toScreenX(locations[path[0]].x);
        int startY = toScreenY(locations[path[0]].y);
        fillellipse(startX, startY, 6, 6); // Perkecil titik awal
        
        setcolor(MAGENTA);
        setfillstyle(SOLID_FILL, MAGENTA);
        int endX = toScreenX(locations[path.back()].x);
        int endY = toScreenY(locations[path.back()].y);
        fillellipse(endX, endY, 6, 6); // Perkecil titik tujuan
        
        // Reset line style
        setlinestyle(SOLID_LINE, 0, 1);
    }
    
    void displayLocations() {
        cout << "\n=== DAFTAR LOKASI PENTING DI SEKITAR SMP ===" << endl;
        cout << "--------------------------------------------" << endl;
        for (int i = 0; i < numLocations; i++) {
            cout << setw(2) << i + 1 << ". " << locations[i].name 
                 << " (" << locations[i].x << ", " << locations[i].y << ")" << endl;
        }
        cout << "\nTekan Enter untuk melanjutkan...";
        cin.ignore();
        cin.get();
    }
    
    vector<int> dijkstra(int start, int end, vector<double>& distances) {
        vector<double> dist(numLocations, numeric_limits<double>::infinity());
        vector<int> prev(numLocations, -1);
        vector<bool> visited(numLocations, false);
        
        priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> pq;
        
        dist[start] = 0;
        pq.push({0, start});
        
        while (!pq.empty()) {
            double d = pq.top().first;
            int u = pq.top().second;
            pq.pop();
            
            if (visited[u]) continue;
            visited[u] = true;
            
            if (u == end) break;
            
            for (const Edge& edge : graph[u]) {
                int v = edge.to;
                double weight = edge.weight;
                
                if (!visited[v] && dist[u] + weight < dist[v]) {
                    dist[v] = dist[u] + weight;
                    prev[v] = u;
                    pq.push({dist[v], v});
                }
            }
        }
        
        distances = dist;
        
        vector<int> path;
        int current = end;
        while (current != -1) {
            path.push_back(current);
            current = prev[current];
        }
        
        reverse(path.begin(), path.end());
        
        if (path.size() == 1 && path[0] != start) {
            path.clear();
        }
        
        return path;
    }
    
    void showGraphicalPath() {
        int start, end;
        
        cout << "\n=== PENCARIAN JALUR TERPENDEK (GRAFIS) ===" << endl;
        displayLocations();
        
        cout << "Masukkan nomor lokasi awal (1-" << numLocations << "): ";
        cin >> start;
        start--; 
        
        cout << "Masukkan nomor lokasi tujuan (1-" << numLocations << "): ";
        cin >> end;
        end--; 
        
        if (start < 0 || start >= numLocations || end < 0 || end >= numLocations) {
            cout << "Nomor lokasi tidak valid!" << endl;
            return;
        }
        
        if (start == end) {
            cout << "Lokasi awal dan tujuan sama!" << endl;
            return;
        }
        
        vector<double> distances;
        vector<int> path = dijkstra(start, end, distances);
        
        if (path.empty()) {
            cout << "\nTidak ada jalur yang ditemukan!" << endl;
            cout << "Lokasi mungkin tidak terhubung dalam jaringan jalan." << endl;
            cout << "Tekan Enter untuk melanjutkan...";
            cin.ignore();
            cin.get();
            return;
        }
        
        // Tampilkan hasil di console
        cout << "\nJalur terpendek ditemukan!" << endl;
        cout << "Jarak total: " << fixed << setprecision(2) << (distances[end] * UNIT_TO_KM) << " KM" << endl;
        cout << "Jumlah titik yang dilalui: " << path.size() << " lokasi" << endl;
        cout << "Dari: " << locations[start].name << endl;
        cout << "Ke: " << locations[end].name << endl;
        cout << "\nJalur mengikuti jalan yang tersedia (bukan garis lurus)" << endl;
        cout << "Membuka tampilan grafis..." << endl;
        
        // Tampilkan grafis
        initializeGraphics();
        drawMap();
        drawPath(path);
        
        // Informasi tambahan di layar grafis (pindahkan ke pojok kanan bawah)
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 2);
        setcolor(WHITE);
        char info[150];
        int baris = 0;
        int fontHeight = textheight(const_cast<char*>("A"));
        int infoX = WINDOW_WIDTH - 500; // Lebar blok keterangan
        int infoY = WINDOW_HEIGHT - (fontHeight * 9) - 30; // 9 baris, 30px margin bawah
        sprintf(info, "Jarak Total: %.2f KM", distances[end] * UNIT_TO_KM);
        outtextxy(infoX, infoY + baris * fontHeight, info); baris++;
        sprintf(info, "Titik Dilalui: %d lokasi", (int)path.size());
        outtextxy(infoX, infoY + baris * fontHeight, info); baris++;
        sprintf(info, "Dari: %s", locations[start].name.c_str());
        outtextxy(infoX, infoY + baris * fontHeight, info); baris++;
        sprintf(info, "Ke: %s", locations[end].name.c_str());
        outtextxy(infoX, infoY + baris * fontHeight, info); baris++;
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 2); // Perbesar font keterangan berikut
        setcolor(YELLOW);
        outtextxy(infoX, infoY + baris * fontHeight, const_cast<char*>("Jalur Terpendek")); baris++;
        setcolor(GREEN);
        outtextxy(infoX, infoY + baris * fontHeight, const_cast<char*>("Titik Awal")); baris++;
        setcolor(MAGENTA);
        outtextxy(infoX, infoY + baris * fontHeight, const_cast<char*>("Titik Tujuan")); baris++;
        setcolor(WHITE);
        outtextxy(infoX, infoY + baris * fontHeight, const_cast<char*>("CATATAN: Jalur mengikuti jalan"));
        baris++;
        outtextxy(infoX, infoY + baris * fontHeight, const_cast<char*>("tersedia dgn algo Dijkstra"));
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 1); // Kembalikan font ke default
        
        getch();
        closegraph();
    }
    
    void showGraphicalMap() {
        cout << "\nMembuka peta grafis..." << endl;
        initializeGraphics();
        drawMap();
        getch();
        closegraph();
    }
    
    void findShortestPath() {
        int start, end;
        
        cout << "\n=== PENCARIAN JALUR TERPENDEK ===" << endl;
        cout << "Masukkan nomor lokasi awal (1-" << numLocations << "): ";
        cin >> start;
        start--; 
        
        cout << "Masukkan nomor lokasi tujuan (1-" << numLocations << "): ";
        cin >> end;
        end--; 
        
        if (start < 0 || start >= numLocations || end < 0 || end >= numLocations) {
            cout << "Nomor lokasi tidak valid!" << endl;
            return;
        }
        
        if (start == end) {
            cout << "Lokasi awal dan tujuan sama!" << endl;
            return;
        }
        
        cout << "\nMencari jalur terpendek dari " << locations[start].name 
             << " ke " << locations[end].name << "..." << endl;
        cout << "Menggunakan algoritma Dijkstra dengan koneksi realistis..." << endl;
        
        vector<double> distances;
        vector<int> path = dijkstra(start, end, distances);
        
        if (!path.empty()) {
            cout << "\n=== JALUR TERPENDEK DITEMUKAN ===" << endl;
            cout << "Jarak total: " << fixed << setprecision(2) << (distances[end] * UNIT_TO_KM) << " KM" << endl;
            cout << "Jumlah lokasi yang dilalui: " << path.size() << " titik" << endl;
            cout << "Jalur yang dilalui:" << endl;
            
            double totalDistance = 0;
            for (int i = 0; i < path.size(); i++) {
                cout << (i + 1) << ". " << locations[path[i]].name;
                if (i < path.size() - 1) {
                    double segmentDistance = calculateDistance(locations[path[i]], locations[path[i + 1]]);
                    totalDistance += segmentDistance;
                    cout << "\n   -> Jarak ke titik berikutnya: " << fixed << setprecision(2) << (segmentDistance * UNIT_TO_KM) << " KM";
                }
                cout << endl;
            }
            
            cout << "\nCATATAN: Jalur ini mengikuti jalan yang tersedia," << endl;
            cout << "         tidak menggunakan garis lurus langsung." << endl;
            
            // Tampilkan koneksi langsung yang tersedia dari titik awal
            cout << "\nKoneksi langsung dari " << locations[start].name << ":" << endl;
            for (const Edge& edge : graph[start]) {
                cout << "  - " << locations[edge.to].name 
                     << " (jarak: " << fixed << setprecision(2) << (edge.weight * UNIT_TO_KM) << " KM)" << endl;
            }
            
        } else {
            cout << "\nTidak ada jalur yang ditemukan!" << endl;
            cout << "Kemungkinan penyebab:" << endl;
            cout << "1. Lokasi tidak terhubung dalam jaringan jalan" << endl;
            cout << "2. Jarak terlalu jauh untuk koneksi langsung" << endl;
            
            // Tampilkan koneksi yang tersedia
            cout << "\nKoneksi yang tersedia dari " << locations[start].name << ":" << endl;
            if (graph[start].empty()) {
                cout << "  - Tidak ada koneksi langsung" << endl;
            } else {
                for (const Edge& edge : graph[start]) {
                    cout << "  - " << locations[edge.to].name 
                         << " (jarak: " << fixed << setprecision(2) << (edge.weight * UNIT_TO_KM) << " KM)" << endl;
                }
            }
        }
        
        cout << "\nTekan Enter untuk melanjutkan...";
        cin.ignore();
        cin.get();
    }
    
    void showRoadConnections() {
        cout << "\n=== JARINGAN JALAN PER LOKASI ===" << endl;
        cout << "Menampilkan koneksi jalan yang tersedia dari setiap lokasi" << endl;
        cout << "=========================================================" << endl;
        
        for (int i = 0; i < numLocations; i++) {
            cout << "\n" << (i + 1) << ". " << locations[i].name << endl;
            cout << "   Koordinat: (" << locations[i].x << ", " << locations[i].y << ")" << endl;
            cout << "   Terhubung langsung ke:" << endl;
            
            if (graph[i].empty()) {
                cout << "   - Tidak ada koneksi langsung" << endl;
            } else {
                // Urutkan koneksi berdasarkan jarak
                vector<pair<double, int>> sortedConnections;
                for (const Edge& edge : graph[i]) {
                    sortedConnections.push_back({edge.weight, edge.to});
                }
                sort(sortedConnections.begin(), sortedConnections.end());
                
                for (int j = 0; j < sortedConnections.size(); j++) {
                    int targetIndex = sortedConnections[j].second;
                    double distance = sortedConnections[j].first;
                    cout << "   " << (j + 1) << ") " << locations[targetIndex].name 
                         << " (jarak: " << fixed << setprecision(2) << (distance * UNIT_TO_KM) << " KM)" << endl;
                }
            }
        }
        
        cout << "\n=========================================================" << endl;
        cout << "CATATAN:" << endl;
        cout << "- Setiap lokasi hanya terhubung dengan maksimal 5 lokasi terdekat" << endl;
        cout << "- Jarak maksimal koneksi langsung: 2000 unit" << endl;
        cout << "- Ini mensimulasikan jaringan jalan yang realistis" << endl;
        cout << "- Untuk mencapai lokasi yang tidak terhubung langsung," << endl;
        cout << "  harus melewati lokasi perantara" << endl;
        
        cout << "\nTekan Enter untuk melanjutkan...";
        cin.ignore();
        cin.get();
    }
    
    void run() {
        int choice;
        
        cout << "===============================================" << endl;
        cout << "    GOOGLE MAPS SIMULATOR VERSI 0.01         " << endl;
        cout << "    Tugas Algoritma Dijkstra untuk SMP       " << endl;
        cout << "    Dengan BGI Graphics Support              " << endl;
        cout << "===============================================" << endl;
        
        while (true) {
            cout << "\n=== MENU UTAMA ===" << endl;
            cout << "1. Tampilkan daftar lokasi" << endl;
            cout << "2. Tampilkan peta grafis (dengan jaringan jalan)" << endl;
            cout << "3. Cari jalur terpendek (teks)" << endl;
            cout << "4. Cari jalur terpendek (grafis)" << endl;
            cout << "5. Lihat koneksi jalan per lokasi" << endl;
            cout << "6. Keluar" << endl;
            cout << "Pilih menu (1-6): ";
            cin >> choice;
            
            switch (choice) {
                case 1:
                    displayLocations();
                    break;
                case 2:
                    showGraphicalMap();
                    break;
                case 3:
                    findShortestPath();
                    break;
                case 4:
                    showGraphicalPath();
                    break;
                case 5:
                    showRoadConnections();
                    break;
                case 6:
                    cout << "\nTerima kasih telah menggunakan Google Maps Simulator!" << endl;
                    cout << "Program dibuat dengan algoritma Dijkstra untuk tugas SMP." << endl;
                    cout << "Fitur: Jalur realistis mengikuti jalan tersedia" << endl;
                    cout << "Deadline: 8 Juni 2025" << endl;
                    return;
                default:
                    cout << "Pilihan tidak valid! Silakan pilih 1-6." << endl;
                    break;
            }
        }
    }
};

int main() {
    GoogleMapsSimulator simulator;
    simulator.run();
    return 0;
}