import pandas as pd
import matplotlib.pyplot as plt
import os
import sys

# Получаем путь к папке с результатами из аргументов (или текущая папка)
output_dir = sys.argv[1] if len(sys.argv) > 1 else "."

# Убедимся, что папка существует
if not os.path.exists(output_dir):
    print(f"Ошибка: Директория {output_dir} не найдена.")
    sys.exit(1)

# Яркие цвета для различных компонент
comp_colors = ['tab:red', 'tab:purple', 'tab:brown', 'tab:pink', 'tab:cyan', 'tab:olive']

# Проходимся по всем CSV в папке
for filename in os.listdir(output_dir):
    if not filename.endswith(".csv"):
        continue
        
    filepath = os.path.join(output_dir, filename)
    df = pd.read_csv(filepath)
    test_name = filename.replace(".csv", "").upper()
    
    # -------------------------------------------------------------
    # 1. ОБЩИЙ ГРАФИК (Все методы на одной картинке)
    # -------------------------------------------------------------
    plt.figure(figsize=(10, 6))
    plt.scatter(df['x'], df['emp'], s=15, alpha=0.3, label='Эмпирическая гистограмма', color='steelblue')
    plt.plot(df['x'], df['true_pdf'], linewidth=2.5, label='Истинное', color='black')
    plt.plot(df['x'], df['nonrob_pdf'], linewidth=2, label='Неробастная', color='darkorange', linestyle='--')
    plt.plot(df['x'], df['rob_pdf'], linewidth=2, label='Робастная', color='forestgreen', linestyle='-.')
    
    plt.title(f"{test_name} - Общее сравнение")
    plt.xlabel("x")
    plt.ylabel("Плотность")
    plt.legend()
    plt.grid(True, linestyle=':', alpha=0.7)
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, f"{test_name}_general.png"), dpi=150)
    plt.close()

    # -------------------------------------------------------------
    # 2. ДЕТАЛИЗАЦИЯ: НЕРОБАСТНЫЙ МЕТОД (Сабплоты)
    # -------------------------------------------------------------
    nr_cols = [c for c in df.columns if c.startswith('nr_c')]
    num_subplots_nr = len(nr_cols) + 1
    
    fig, axes = plt.subplots(num_subplots_nr, 1, figsize=(10, 3 * num_subplots_nr), sharex=True)
    if num_subplots_nr == 1:
        axes = [axes]

    # Верхний график: Общая картина
    axes[0].scatter(df['x'], df['emp'], s=10, alpha=0.3, color='steelblue', label='Эмпирика')
    axes[0].plot(df['x'], df['nonrob_pdf'], linewidth=2.5, color='darkorange', label='Суммарная неробастная модель')
    axes[0].set_title(f"{test_name} - Неробастная модель (Разложение по компонентам)")
    axes[0].legend(loc="upper right")
    axes[0].grid(True, linestyle=':', alpha=0.7)

    # Нижние графики: Отдельные компоненты
    for i, col in enumerate(nr_cols):
        ax = axes[i + 1]
        c_color = comp_colors[i % len(comp_colors)]
        
        ax.fill_between(df['x'], df['emp'], alpha=0.1, color='steelblue')
        ax.plot(df['x'], df[col], linewidth=2.5, color=c_color, label=f"Компонента {i+1} (Нормальная)")
        ax.legend(loc="upper right")
        ax.set_ylabel("Плотность")
        ax.grid(True, linestyle=':', alpha=0.7)
    
    axes[-1].set_xlabel("x")
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, f"{test_name}_nonrobust_components.png"), dpi=150)
    plt.close()

    # -------------------------------------------------------------
    # 3. ДЕТАЛИЗАЦИЯ: РОБАСТНЫЙ МЕТОД (Сабплоты)
    # -------------------------------------------------------------
    r_cols = [c for c in df.columns if c.startswith('r_c')]
    num_subplots = len(r_cols) + 1
    
    fig, axes = plt.subplots(num_subplots, 1, figsize=(10, 3 * num_subplots), sharex=True)
    if num_subplots == 1:
        axes = [axes]

    # Верхний график: Общая картина
    axes[0].scatter(df['x'], df['emp'], s=10, alpha=0.3, color='steelblue', label='Эмпирика')
    axes[0].plot(df['x'], df['rob_pdf'], linewidth=2.5, color='forestgreen', label='Суммарная робастная модель')
    axes[0].set_title(f"{test_name} - Робастная модель (Разложение по компонентам)")
    axes[0].legend(loc="upper right")
    axes[0].grid(True, linestyle=':', alpha=0.7)

    # Нижние графики: Отдельные компоненты
    for i, col in enumerate(r_cols):
        ax = axes[i + 1]
        c_color = comp_colors[i % len(comp_colors)]
        
        # Заливаем гистограмму на фон для ориентира
        ax.fill_between(df['x'], df['emp'], alpha=0.1, color='steelblue')
        
        # Выделяем равномерный шум красным цветом
        is_noise = (i == len(r_cols) - 1)
        if is_noise:
            c_color = 'crimson'
            lbl = "Равномерный шум (Робастная добавка)"
        else:
            lbl = f"Компонента {i+1} (Нормальная)"
        
        ax.plot(df['x'], df[col], linewidth=2.5, color=c_color, label=lbl)
        ax.legend(loc="upper right")
        ax.set_ylabel("Плотность")
        ax.grid(True, linestyle=':', alpha=0.7)
    
    axes[-1].set_xlabel("x")
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, f"{test_name}_robust_components.png"), dpi=150)
    plt.close()

print(f"Построение цветных графиков завершено! Сохранено в папке: {output_dir}")