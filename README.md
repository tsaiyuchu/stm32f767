# stm32f767
the ll course
# STM32F767ZIT6 Nucleo144 LL
## Download 
1. STM32CubeMX
2. STM32CubeIDE
3. Drivers_STM32CubeF7(https://www.st.com/en/embedded-software/stm32cubef7.html)
4. stm32fxxx Reference manual/ STM32F76_datasheet/ stm32f767-nucleo144-user-manual/stm32f7-hal-and-lowlayer-drivers

---
## 1_GPIO
![image](https://github.com/user-attachments/assets/dd639421-7dbd-4ba3-9f42-f0601fb0e5f4)
Drivers\STM32F7xx_HAL_Driver\Inc
Drivers\CMSIS\Include
Drivers\CMSIS\Device\ST\STM32F7xx\Include
![image](https://github.com/user-attachments/assets/9c4dae2a-8340-4c3e-bd31-4cc65194f9c2)
STM32F767xx
USE_FULL_LL_DRIVERS

## 4_Universal Asynchronous Receiver/Transmitter (UART)
通用非同步接受/傳輸器
### 4.1  Serial vs. Parallel
![image](https://github.com/user-attachments/assets/7dd8dfe0-00cf-434b-8a75-1978542b316f)
1. Serial(串形接口)：傳輸一位元一次，適合長距離傳輸。
2. Parallel(並形接口)：傳輸多位元一次，適合短距離且需要高速度的情況。
### 4.2 Synchronous vs. Asynchronous
![image](https://github.com/user-attachments/assets/c4aaadd6-5a98-4810-a39e-1ddfd72843b6)
1. Synchronous 同步通訊：依靠額外的時鐘信號來同步數據，傳輸速度更快且穩定，但需要更多的線路。
   發送器和接收器依賴共享的時鐘來同步數據的傳輸。
   因為有時鐘信號，數據的傳輸速度和準確性較高。
   適合需要快速且穩定的數據傳輸場景。
3. Asynchronous 非同步通訊：依靠起始位和停止位來達成同步，線路簡單，但傳輸速度較慢，且需要事先設定波特率。
   發送器和接收器事先需要約定波特率（Baud Rate），即每秒傳輸的比特數，來同步數據的解析。
   為了確保數據準確性，數據流中會加入起始位（Start Bit）和停止位（Stop Bit）。
   雖然不需要額外的時鐘信號線，但可能會受到頻率偏移（Drift）的影響。
![image](https://github.com/user-attachments/assets/e0212ca6-8946-4b03-b8e0-eff3b256404e)
### 4.3 Uart vs. Usart
![image](https://github.com/user-attachments/assets/725b1941-2d6d-4a63-ab78-c50a158bedbb)
1. UART: 僅支援非同步通訊
   用於非同步通訊，數據的傳輸和接收不需要共享時鐘信號。
   通常依靠起始位（Start Bit）和停止位（Stop Bit）來同步數據的傳輸。
2. USART：既支援同步通訊，也支援非同步通訊，功能比 UART 更強大。
  同步模式：需要發送時鐘信號，數據和時鐘一起傳輸，傳輸速度更快。
  非同步模式：類似於 UART，只傳輸數據，依靠波特率匹配來進行同步。
### 4.4 Transmission Mode
![image](https://github.com/user-attachments/assets/292b5c8b-8a5b-48e1-81b6-367d0451d7cc)
1. Duplex（雙工)：數據可以同時傳輸和接收。  
2. Simplex（單工）:數據只能以一個方向傳輸，也就是只有「發送」或「接收」中的一個。 
3. Half Duplex（半雙工）:數據只能在一個方向上傳輸，但可以切換方向。一次只能傳輸或接收數據，而不能同時進行。 
4. Full Duplex（全雙工）:數據可以在兩個方向上同時傳輸。  
### 4.5 Protocol Summary
![image](https://github.com/user-attachments/assets/7be598e5-a6ba-4f5d-bb09-724bac11a1e8)
1. Start Bit：
   - **長度**：固定為 1 位元（bit）。
   - **值**：永遠為 0，表示數據的開始。
   - **作用**：通知接收端即將開始傳輸數據。

2. **Data Bits (數據位)**：
   - 包含實際的數據內容，例如 ASCII 字符的二進制表示。
   - 以 ASCII 字符 'A' 為例，其二進制表示為 `0100 0001`。

3. **Stop Bit (停止位)**：
   - **長度**：可以是 1 或 2 位元（bit）。
   - **值**：永遠為 1。
   - **作用**：表示數據傳輸結束，並讓接收端準備好處理下一個數據幀。
   - 
範例解釋：
- 傳輸 ASCII 字符 `'A'`，其二進制為 `0100 0001`，在 UART 協議下的一個完整幀包含以下部分：
  - **Start Bit**：`0`（1 位）。
  - **Data Bits**：`0100 0001`（8 位）。
  - **Stop Bit(s)**：`11`（2 位，這裡選用 2 位作為範例）。

- 整個數據幀為：`0 0100 0001 11`。

### 4.6 Configuration Parameters
![image](https://github.com/user-attachments/assets/598ae544-c238-49de-be76-424802a875af)

1. **Baudrate（波特率）**
- **定義**：表示通訊速度，以每秒傳輸的位數（bits per second, bps）為單位。
- **作用**：
  - 需在發送端和接收端設定相同的波特率，否則會導致數據無法正確傳輸。
  - 常見的波特率有：9600、115200 等。
- **影響**：
  - 波特率越高，數據傳輸速度越快，但對硬體的精準時序要求也越高。

---

2. **Stop Bit（停止位）**
- **定義**：數據傳輸完成後，用於表示結束的位元數，可以是 **1 位** 或 **2 位**。
- **作用**：
  - 告知接收端當前數據幀已完成，讓接收端準備好下一個幀。
- **配置選擇**：
  - 默認情況下大多使用 **1 位** 停止位；在某些情況下（如高干擾環境）可以選擇 **2 位** 停止位，以提高數據穩定性。

---

3. **Parity（校驗位）**
- **定義**：表示數據幀的奇偶校驗模式，用於檢測數據傳輸中的錯誤。
- **類型**：
  - **Odd（奇校驗）**：校驗位設為 `1`，當數據位中 `1` 的個數為奇數。
  - **Even（偶校驗）**：校驗位設為 `1`，當數據位中 `1` 的個數為偶數。
  - **None（無校驗）**：不使用校驗位（傳輸效率最高，但無法檢查錯誤）。
- **作用**：
  - 提高通訊的可靠性，檢測和避免簡單的數據錯誤。
這張圖進一步說明了 **UART 配置參數**，包括傳輸模式、數據長度和硬體流控的設定，以下是每個參數的解釋：

---

4. **Mode（模式）**
- **定義**：指定 UART 是否啟用接收模式（RX）或傳輸模式（TX）。
- **作用**：
  - RX（接收模式）：啟用接收功能，允許從另一端接收數據。
  - TX（傳輸模式）：啟用傳輸功能，允許向另一端發送數據。
  - RX 和 TX 可以分別啟用，也可以同時啟用（雙工模式）。
- **應用場景**：根據應用需求選擇只傳輸、只接收，或同時進行。

---

5. **Word Length（數據位長度）**
- **定義**：指定每次傳輸的數據位元數，通常可以設定為 **8 位** 或 **9 位**。
- **作用**：
  - 8 位：是 UART 的常見默認設置，適合大多數應用。
  - 9 位：用於特殊應用場合，允許傳輸更多數據或進行擴展。
- **影響**：發送端和接收端的數據位長度必須一致，否則可能導致數據解析錯誤。

---

6. **Hardware Flow Control（硬體流量控制）**
- **定義**：指定是否啟用硬體流控。
- **作用**：
  - 啟用硬體流控時，使用附加的控制信號（如 RTS/CTS）來管理數據流量，防止數據丟失或溢出。
  - 如果未啟用，流控由軟體協議（如 XON/XOFF）管理，效率較低。
- **優點**：
  - 硬體流控可確保高速和穩定的數據傳輸，特別是在大數據量傳輸時。
## 5_Inter-Integrated Circuit (I2C)
![image](https://github.com/user-attachments/assets/1c266bd2-0132-4ce7-b9f8-809f3c77f2c1)
1. THE PROTOCOL
   I2C 是一種 **兩線接口**，SCL 和 SDA 是其核心，這也是名稱 "Two-Wire Interface" 的由來。
   它的設計允許多個主設備與從設備共享同一條總線。
   **I2C線路的組成**：
   - **SCL (Serial Clock)**：
     - 用於在主設備 (Master) 和從設備 (Slave) 之間同步數據傳輸的時鐘信號線。
   - **SDA (Serial Data)**：
     - 數據線，用於在設備之間進行實際數據的傳輸。

   **電路圖**：
   - 圖片中顯示了 **SDA** 和 **SCL** 兩條線，連接多個主設備和多個從設備。
   - **Rp (Pull-up Resistors)**：
     - SCL 和 SDA 線需要透過上拉電阻 (Rp) 連接到電源 (VDD) 以保持穩定的高電平。
操作模式：
Master Transmitter：主設備作為發送方，向從設備傳輸數據。
Master Receiver：主設備作為接收方，從從設備接收數據。
Slave Transmitter：從設備作為發送方，將數據傳輸給主設備。
Slave Receiver：從設備作為接收方，接收來自主設備的數據。
 **I2C 通訊協議 (The Protocol)** 的關鍵特性。以下是重點內容的解釋：

**主設備控制交易 (Transactions)**：
   - 所有的數據傳輸都是由主設備 (Master) 發起並完成的。主設備負責管理總線的使用權，開始和結束通信。

**消息結構 (Message Structure)**：
   - 每個消息 (Message) 包含兩個主要部分：
     - **地址幀 (Address Frame)**：用於指定從設備的唯一地址。
     - **數據幀 (Data Frame)**：傳輸具體數據的內容。
**數據的時序 (Data Timing)**：
   - 數據會在 **SCL (Serial Clock)** 線變低電平後放置到 **SDA (Serial Data)** 線上。
   - 數據會在 **SCL 線變高電平後被採樣**，這確保了數據的同步和正確性。
2. START AND STOP CONDITION
![image](https://github.com/user-attachments/assets/4b34932d-48c8-4a3c-aa62-a4e34bc7f7f4)
 **START AND STOP**：
   - 所有 I2C 通訊以 **START 條件** 開始，並以 **STOP 條件** 結束。

2. **START 條件**：
   - 當 **SCL 線維持高電平** 時，**SDA 線從高電平轉為低電平**，這定義為 START 條件。
   - 這是一個信號，表明主設備準備開始通信。

3. **STOP 條件**：
   - 當 **SCL 線維持高電平** 時，**SDA 線從低電平轉為高電平**，這定義為 STOP 條件。
   - 這是一個信號，表明主設備結束了當前的通信。

4. **主設備的控制**：
   - START 和 STOP 條件始終由主設備產生。
   - 當 STOP 條件發生後，總線會在一定時間後被視為空閒。

5. **總線狀態**：
   - 如果在 STOP 條件之前產生了另一個 START 條件，則總線會保持忙碌狀態。
   - 這種情況被稱為 **重複 START (Repeated START)**，用於連續交易而不釋放總線。

6. **數據傳輸格式**：
   - 一個完整的 I2C 通訊幀，包括 START、從設備地址 (Slave Address)、讀寫位 (R/W)、數據 (Data) 和 STOP。

