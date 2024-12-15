from modules.rabin_karp import rabin_karp
from modules.graham import *

def main():
    print("Выберите алгоритм для выполнения:")
    print("1. Алгоритм поиска подстроки Rabin-Karp")
    print("2. Алгоритм выпуклой оболочки Грэма")

    choice = input("Введите 1 или 2: ").strip()

    if choice == "1":
        pattern = input("Введите шаблон: ").strip()
        text = input("Введите текст: ").strip()

        result = rabin_karp(pattern, text)
        print("Шаблон найден в позициях:", " ".join(map(str, result)))

    elif choice == "2":
        n = int(input("Введите количество точек: "))
        points = []
        for _ in range(n):
            x, y = map(int, input("Введите координаты точки (x, y): ").split(", "))
            points.append(Point(x, y))

        hull = graham(points)
        area = calculate_area(hull)
        print(f"Выпуклая оболочка: {hull}")
        print(f"Площадь выпуклой оболочки: {area:.2f}")

        # Визуализация
        visualize(points, hull)

    else:
        print("Неверный выбор. Пожалуйста, введите 1 или 2.")

if __name__ == "__main__":
    main()
