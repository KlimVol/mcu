import time
import serial
import matplotlib.pyplot as plt

def get_numeric_value(ser, command):
    ser.write(f"{command}\n".encode('ascii'))
    for _ in range(5): # Проверяем до 5 строк ответа
        line = ser.readline().decode('ascii').strip()
        if not line:
            continue
        try:
            val = float(line)
            return val
        except ValueError:
            continue
    return None

def main():
    ser = serial.Serial(port='COM6', baudrate=115200, timeout=0.5)
    
    measure_t = []
    measure_p = []
    measure_h = []
    measure_ts = []
    
    start_ts = time.time()
    print("Сбор данных запущен. Нажмите Ctrl+C для остановки и вывода графиков.")

    try:
        while True:
            ts = time.time() - start_ts
            
            # Опрашиваем датчик по очереди
            t = get_numeric_value(ser, "temp")
            p = get_numeric_value(ser, "pres")
            h = get_numeric_value(ser, "hum")

            if None not in (t, p, h):
                measure_ts.append(ts)
                measure_t.append(t)
                measure_p.append(p)
                measure_h.append(h)
                print(f"[{ts:6.2f}s] T: {t:5.2f}C | P: {p:8.2f}Pa | H: {h:5.2f}%")
            
            time.sleep(0.1) # Частота опроса ~10 Гц

    except KeyboardInterrupt:
        print("\nСбор данных остановлен. Рисую графики...")
    finally:
        ser.close()

        # Построение графиков
        fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(10, 10))
        
        ax1.plot(measure_ts, measure_t, 'r-')
        ax1.set_title('Температура (°C)')
        ax1.grid(True)

        ax2.plot(measure_ts, measure_p, 'g-')
        ax2.set_title('Давление (Па)')
        ax2.grid(True)

        ax3.plot(measure_ts, measure_h, 'b-')
        ax3.set_title('Влажность (%)')
        ax3.set_xlabel('Время (с)')
        ax3.grid(True)

        plt.tight_layout()
        plt.show()

if __name__ == "__main__":
    main()
