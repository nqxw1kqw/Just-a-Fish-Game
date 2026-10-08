# 躍る (Odoru): Tài liệu thiết kế & kỹ thuật

> [!INFO] 🔗 Mạng lưới Liên kết Ghi chú (Obsidian Vault Links)
> - 🏛️ **Kiến trúc Hiện tại (Bản mới nhất)**: [[Odoru_Current_Architecture|Odoru_Current_Architecture.md]]
> - 📑 **Hồ sơ Đặc tả Kỹ thuật Chi tiết**: [[Odoru_Technical_Specification|Odoru_Technical_Specification.md]]
> - 📌 **Hướng dẫn Dự án & Phím tắt**: [[README|README.md]]

## 1. Tổng quan

- **Thể loại**: 3D arena survivor / roguelite nâng cấp liên tục (kiểu Brotato)
- **Theme**: 躍る. Nghĩa đen là cá nảy, nghĩa bóng là 胸躍る (mạnh lên thấy rõ, chiến đấu hưng phấn)
- **Công nghệ**: C++, DxLib, Visual Studio (không dùng engine)
- **Camera**: cố định, nghiêng khoảng 50 độ để thấy rõ độ cao và bóng
- **Ý tưởng cốt lõi**: cá mắc cạn tự nảy theo hướng người chơi, súng gắn liền với cá tự bắn, nâng cấp xoay quanh nhịp nảy

---

## 2. Thiết kế game

### 2.1 Cá chính

| Hạng mục | Thiết kế |
|---|---|
| Di chuyển | Tự nảy khi có input hướng, không cần nút. Thả input thì nảy nốt rồi dừng |
| Hướng | Đổi hoàn toàn khi chạm đất, chỉnh nhẹ khi đang bay (air control yếu) |
| Nhịp nảy | Khoảng 0.3 đến 0.5 giây mỗi lần |
| Súng | Gắn liền với cá, tự ngắm quái gần nhất, tự bắn |
| Đặc quyền | Bất khả xâm phạm ở gần đỉnh nảy (chỉ cá chính có) |
| Tiếp đất | Sóng nước gây sát thương nhỏ, đẩy lùi quái |

### 2.2 Vòng lặp

1. Vào wave, sinh quái theo đợt
2. Cá nảy, né, tự bắn, nhặt tiền/exp
3. Hết wave thì vào shop chọn nâng cấp
4. Wave tiếp theo khó hơn, có boss ở các mốc
5. Chết hoặc thắng thì tổng kết, bắt đầu lại

### 2.3 Hệ thống nâng cấp

| Nhóm | Ví dụ |
|---|---|
| Chỉnh súng cơ bản | Sát thương, tốc độ bắn, số tia, xuyên, nảy đạn, chí mạng |
| Ăn theo nhịp nảy | Bắn nhanh hơn trên không, mưa đạn khi tiếp đất, combo nảy liên hoàn, sóng nước to hơn, nảy cao tăng đạn mạnh |
| Đổi kiểu bắn | Đạn thường, tia nước áp lực, gai tỏa tròn (cá nóc), ngư lôi, sét |
| Hiệu ứng trạng thái | Làm chậm, đóng băng, độc, choáng |
| Tiến hóa (飛躍) | Súng và ngoại hình cá đổi theo cấp (cá con, cá chép, cá rồng), có synergy |

**Nguyên tắc cân bằng**: nâng cấp "nảy nhanh hơn" tăng cả di chuyển, tốc độ bắn và sóng nước nên rất mạnh. Cần giá cao, giới hạn cấp, hoặc đánh đổi (nảy nhanh thì thấp hơn).

### 2.4 Kẻ địch

| Loài | Di chuyển | Vai trò |
|---|---|---|
| Cá nhỏ | Nảy chậm, đi bầy đông | Quái lính |
| Cua / tôm | Nảy ngắn hoặc chạy ngang | Quấy rối |
| Rùa | Bò chậm, giáp cứng | Đòi sát thương đặc biệt, bắn lúc thò đầu |
| Sứa | Lơ lửng, chạm vào bị tê liệt | Không bị sóng nước đẩy, buộc phải bắn |
| Cá mập / cá đuối | Lao thẳng nhanh | Buộc người chơi né |
| Bạch tuộc | Đứng yên hoặc bò, bắn mực | Che tầm nhìn |
| Cá voi | Trườn nặng, thỉnh thoảng nhảy một cú lớn có báo hiệu | Mini-boss |

Quy tắc: quái nảy vẫn trúng đạn bình thường khi ở trên không. Chỉ cá chính có bất khả xâm phạm.

---

## 3. Kiến trúc kỹ thuật

### 3.1 Cấu trúc thư mục

```
src/
  main.cpp              // khởi tạo DxLib, vòng lặp chính
  Core/
    Time.*              // dt, fixed timestep, time scale (hit stop)
    Rng.*               // mt19937 có seed
    Input.*             // gom input WASD/pad thành vector hướng
  Game/
    StateMachine.*      // Title, Wave, Shop, Result
    WaveManager.*       // lịch sinh quái
  Entity/
    Fish.*
    Enemy.*             // dữ liệu + hành vi theo loài
    Bullet.*
    Pickup.*            // tiền, exp
    Pool.h              // pool mảng cố định
  System/
    Collision.*         // lưới không gian
    Stats.*             // hệ chỉ số có modifier
    Upgrade.*           // định nghĩa nâng cấp + hook sự kiện
    Targeting.*         // chọn mục tiêu cho súng
    Fx.*                // hit stop, camera shake, particle
    Render.*            // camera, vẽ model, bóng, culling
  Data/
    enemies.csv, upgrades.csv, waves.csv
```

### 3.2 Vòng lặp chính: fixed timestep

Logic chạy bước cố định (ví dụ 1/120 giây), vẽ chạy theo tốc độ màn hình. Lợi ích: kết quả va chạm và nảy giống nhau trên mọi máy, đạn nhanh không xuyên quái, dễ debug và replay.

```cpp
constexpr float kStep = 1.0f / 120.0f;
float acc = 0;
LONGLONG prev = GetNowHiPerformanceCount();

while (ProcessMessage() == 0) {
    LONGLONG now = GetNowHiPerformanceCount();
    float frame = (now - prev) / 1000000.0f;
    prev = now;
    if (frame > 0.1f) frame = 0.1f;          // chặn spiral of death khi lag
    acc += frame * timeScale;                 // timeScale = 0 khi hit stop

    while (acc >= kStep) { Update(kStep); acc -= kStep; }

    Render(acc / kStep);                      // alpha để nội suy khi vẽ
    ScreenFlip();
}
```

### 3.3 Cơ chế nảy

Tách vị trí thành hai phần: **mặt phẳng XZ** (di chuyển thật, dùng cho va chạm) và **độ cao Y** (hiệu ứng parabol, dùng cho vẽ, bóng, bất khả xâm phạm).

Với độ cao nảy `H` và chu kỳ `T`, pha `t` chạy 0 đến 1:

- Độ cao: `y = 4·H·t·(1 - t)`
- Tương đương vật lý: vận tốc đầu `vy0 = 4H/T`, gia tốc `g = 8H/T²`

Dùng công thức theo pha thay vì tích phân vật lý vì ổn định hơn và dễ chỉnh `H`, `T` độc lập.

**Bất khả xâm phạm ở đỉnh**: điều kiện `y > k·H` (ví dụ `k = 0.6`) tương ứng pha `t` trong khoảng khoảng 0.18 đến 0.82. Tức là chỉ khoảng 64% thời gian nảy là an toàn, phần còn lại (sát đất) dễ bị trúng. Chỉnh `k` để cân độ khó.

**Sự kiện theo nhịp** (nền tảng của nhóm nâng cấp "ăn theo nảy"):
- `OnTakeoff`: pha bắt đầu
- `OnApex`: pha qua 0.5
- `OnLand`: pha về 1, sinh sóng nước, đổi hướng

### 3.4 Va chạm

- **Hình dạng**: vòng tròn trên mặt phẳng XZ (cá, quái, đạn, pickup). Không dùng physics engine.
- **Lưới không gian (spatial hash grid)**: kích thước ô khoảng 2 lần bán kính quái lớn nhất. Mỗi tick xếp lại quái vào ô, truy vấn chỉ xét 9 ô quanh. Chi phí khoảng O(n) thay vì O(n²). Với 500 quái, so từng cặp là khoảng 125.000 phép kiểm tra, còn lưới chỉ vài nghìn.
- **Đạn nhanh**: dùng swept test (đoạn thẳng từ vị trí cũ đến mới so với vòng tròn quái) để không xuyên.
- **Quái chen nhau**: lực đẩy mềm. Khi hai quái chồng nhau thì mỗi con bị đẩy ra một phần, tránh tụ thành một đống nhưng vẫn rẻ.
- **Sóng nước**: truy vấn lưới theo bán kính, chỉ quái "dưới đất" bị đẩy (sứa bỏ qua nhờ cờ `flying`).

### 3.5 Object pooling

Mọi thứ sinh nhiều (quái, đạn, pickup, particle) nằm trong mảng cố định kèm cờ `active`:

```cpp
template<class T, size_t N>
struct Pool {
    std::array<T, N> items;
    std::array<bool, N> active{};
    T* Spawn();                  // tìm slot trống (giữ free-list để O(1))
    void Despawn(size_t i);
};
```

Không `new/delete` trong lúc chơi, tránh giật do cấp phát và phân mảnh. Đặt giới hạn cứng (ví dụ 600 quái, 1500 đạn). Khi đầy thì bỏ qua lần sinh mới hoặc ưu tiên quái quan trọng.

### 3.6 Súng tự ngắm

- **Chọn mục tiêu**: quái gần nhất trong tầm. Không tìm mỗi tick mà **chọn lại mỗi 0.1 giây** và giữ mục tiêu giữa các lần, giảm chi phí đáng kể. Dùng lưới để tìm theo vòng từ gần ra xa, dừng khi gặp.
- **Bắn**: bộ đếm hồi chiêu riêng, `fireRate` lấy từ hệ chỉ số. Đạn sinh tại `(x, y + offset, z)` của cá.
- **Đạn chạy trên XZ**: đạn bay ngang ở độ cao cố định để tính va chạm đơn giản, độ cao cá chỉ ảnh hưởng điểm xuất phát và hiệu ứng.
- **Nâng cấp bắn trên không**: tính nhân hệ số tốc độ bắn theo độ cao hiện tại `y/H`.

### 3.7 Hệ chỉ số (Stats)

Mỗi chỉ số có giá trị gốc và danh sách modifier, tính lại khi nâng cấp thay đổi (không tính mỗi frame):

```
final = (base + sum(add)) * (1 + sum(percent)) * product(more)
```

- `add`: cộng thẳng (+2 sát thương)
- `percent`: cộng dồn cùng nhóm (+10% +10% = +20%)
- `more`: nhân riêng, hiếm và mạnh (dùng cho tiến hóa)

Tách ba loại này giúp cân bằng dễ đoán, tránh nâng cấp nhân chồng lên nhau gây bùng nổ.

### 3.8 Nâng cấp theo dữ liệu + hook

Nâng cấp là dòng trong `upgrades.csv` (id, tên, nhóm, modifier, hook), không phải code riêng:

- **Modifier thuần**: chỉ đổi chỉ số, xử lý chung.
- **Hook sự kiện**: gắn vào `OnLand`, `OnApex`, `OnHit`, `OnKill`, `OnWaveStart`. Ví dụ "mưa đạn tiếp đất" đăng ký vào `OnLand` và bắn một vòng đạn.
- Tên dùng tham chiếu id, nội dung hiển thị tách khỏi logic nên sau này dễ đổi tên tiếng Nhật.

Cách này cho phép thêm nâng cấp mới chỉ bằng thêm dòng dữ liệu và một hook nhỏ.

### 3.9 Hành vi kẻ địch

Mỗi loài là một **hàm cập nhật** chọn theo enum `Kind` (switch hoặc bảng hàm), không dùng kế thừa ảo, để dữ liệu nằm liền nhau trong bộ nhớ:

- **Chase**: hướng về cá, tốc độ cố định (cá nhỏ, rùa)
- **Hop**: Chase + pha nảy riêng (dùng chung code nảy với cá chính, chỉ đổi `H`, `T`, tốc độ)
- **Charge**: ngắm, đứng khựng báo hiệu, rồi lao thẳng (cá mập)
- **Float**: trôi chậm, cờ `flying` (sứa)
- **Turret**: đứng yên, bắn theo chu kỳ (bạch tuộc)
- **Slam**: nhảy cú lớn có bóng báo trước, đập xuống vùng lớn (cá voi)

Mọi đòn nguy hiểm phải có **báo hiệu** (bóng phình, nhấp nháy) trước khi gây sát thương để người chơi kịp phản ứng.

### 3.10 Hiển thị với DxLib

- **Z-buffer**: bật `SetUseZBuffer3D(TRUE)` và `SetWriteZBuffer3D(TRUE)`.
- **Camera**: `SetCameraPositionAndTarget_UpVecY`, `SetCameraNearFar`. Camera bám theo cá bằng nội suy mượt (lerp), thêm rung camera bằng cách cộng offset ngẫu nhiên giảm dần vào target.
- **Bóng**: quad dẹt (`DrawPolygon3D`) hoặc hình tròn mờ sát mặt đất tại `(x, 0, z)`. Khi cá lên cao thì bóng nhỏ và nhạt lại. Việc này giúp người chơi đọc được độ cao và điểm rơi.
- **Quái số lượng lớn**:
  - Dùng model low-poly, nhân bản bằng `MV1DuplicateModel`
  - Loại bỏ ngoài tầm nhìn bằng `CheckCameraViewClip`
  - Quái lính có thể dùng billboard (`DrawBillboard3D`) thay model để nhẹ hơn
  - Không dùng animation xương cho quái đông. Dùng **squash & stretch thủ tục** bằng `MV1SetScale` theo pha nảy
- **Sương mù**: `SetFogEnable`, `SetFogStartEnd` che bớt quái ở xa, vừa tạo không khí dưới nước vừa giảm chi phí.
- **Vẽ theo thứ tự**: nền, bóng, quái và cá, đạn, hiệu ứng trong suốt (bật blend riêng và tắt ghi Z cho phần trong suốt), UI cuối cùng.
- **Cá**: model ít xương, đuôi vẫy bằng vertex wobble hoặc xoay thủ tục theo pha nảy.

### 3.11 Cảm giác chơi (game feel)

Đây là phần quyết định "hưng phấn" hơn cả đồ họa:

- **Hit stop**: khi trúng đòn mạnh thì `timeScale = 0` trong 30 đến 60ms, hoặc làm chậm cục bộ.
- **Rung camera**: nhẹ khi tiếp đất, mạnh khi boss đập.
- **Squash & stretch** cho cá và quái theo pha nảy.
- **Số sát thương và tung nước** tại điểm trúng.
- **Âm thanh**: tiếng bắn, tiếng nảy, tiếng nhặt đồ phải có phản hồi riêng (dùng `PlaySoundMem`, nhiều bản sao cho âm lặp nhanh).
- **Hồi quy sức mạnh**: mỗi wave nên có một khoảnh khắc "mình mạnh hơn rõ rệt".

### 3.12 Dữ liệu và công cụ

- **Dữ liệu ngoài**: CSV (hoặc JSON) cho quái, nâng cấp, wave. Tải khi khởi động, có phím tải lại nóng khi debug.
- **Ngẫu nhiên**: `std::mt19937` có seed lưu theo lượt chơi, giúp tái hiện lỗi.
- **Debug overlay**: `DrawFormatString` hiển thị FPS, số quái, số đạn, thời gian từng hệ thống.
- **Lưu game**: file nhị phân hoặc JSON nhỏ cho tiến trình meta (nếu có).

---

## 4. Ngân sách hiệu năng (mục tiêu)

| Mục | Mục tiêu |
|---|---|
| FPS | 60 ổn định |
| Quái cùng lúc | khoảng 300 đến 500 |
| Đạn cùng lúc | khoảng 1000 |
| Logic mỗi tick | dưới khoảng 3ms |
| Bộ nhớ động trong lúc chơi | gần như không cấp phát |

Cần đo sớm bằng prototype với số quái giả (hình cầu) để biết ngưỡng thật của DxLib trước khi đầu tư model.

---

## 5. Lộ trình

1. **Prototype**: cá hình cầu nảy, bóng, 1 súng auto, 1 quái đuổi, lưới va chạm, pool
2. **Vòng chơi**: wave, tiền, shop, khoảng 10 nâng cấp, hệ chỉ số
3. **Mở rộng quái**: rùa, sứa, cá mập, bạch tuộc, rồi cá voi làm boss
4. **Game feel**: hit stop, rung camera, âm thanh, số sát thương
5. **Đồ họa**: model low-poly, sương, tiến hóa cá
6. **Cân bằng và hoàn thiện**

**Điểm kiểm chứng**: sau bước 1, nếu chỉ riêng nảy và bắn đã vui thì tiếp tục. Nếu chưa, chỉnh `H`, `T`, tốc độ và tầm bắn trước khi thêm nội dung.

---

## 6. Rủi ro và cách xử lý

| Rủi ro | Cách xử lý |
|---|---|
| Quái quá đông làm tụt FPS | Lưới không gian, pooling, culling, billboard cho quái lính, giới hạn cứng |
| Khó đọc độ cao trong 3D | Bóng rõ, camera nghiêng, vạch điểm rơi |
| Nảy làm khó né | Air control yếu, nảy ngắn nhanh, đổi hướng khi chạm đất |
| Một nâng cấp thống trị | Giá cao, giới hạn cấp, đánh đổi, cân bằng bằng dữ liệu |
| Cảm giác chơi nhạt | Đầu tư hit stop, âm thanh, rung camera từ sớm |
| Phạm vi quá rộng | Làm bản nhỏ trước (10 nâng cấp, 5 loại quái), mở rộng sau khi thấy vui |
