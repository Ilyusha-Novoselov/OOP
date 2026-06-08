import pandas as pd
import matplotlib.pyplot as plt
import os
import sys

# Получаем путь к папке с результатами из аргументов (или текущая папка)
output_dir = sys.argv[1] if len(sys.argv) > 1 else "."

if not os.path.exists(output_dir):
    print(f"Ошибка: Директория {output_dir} не найдена.")
    sys.exit(1)

# Яркие цвета для различных компонент
comp_colors = ['tab:red', 'tab:purple', 'tab:brown', 'tab:pink', 'tab:cyan', 'tab:olive']

def read_first_line(filepath):
    """Читает первую строку файла, пробуя utf-8, затем cp1251."""
    for enc in ('utf-8', 'cp1251', 'latin-1'):
        try:
            with open(filepath, 'r', encoding=enc) as f:
                return f.readline().strip()
        except UnicodeDecodeError:
            continue
    return ''

def read_title(filepath):
    """Читает строку '# title: ...' из первой строки CSV."""
    first = read_first_line(filepath)
    if first.startswith('# title:'):
        return first[len('# title:'):].strip()
    return None

for filename in sorted(os.listdir(output_dir)):
    if not filename.endswith(".csv"):
        continue

    filepath = os.path.join(output_dir, filename)

    human_title = read_title(filepath)

    # Читаем данные, пропуская строки-комментарии; пробуем обе кодировки
    for enc in ('utf-8', 'cp1251', 'latin-1'):
        try:
            df = pd.read_csv(filepath, comment='#', encoding=enc)
            break
        except UnicodeDecodeError:
            continue

    file_stem = filename.replace(".csv", "").upper()
    display_title = human_title if human_title else file_stem

    print(f"  Строю графики: {display_title}")

    # ----------------------------------------------------------------
    # 1. ОБЩИЙ ГРАФИК
    # ----------------------------------------------------------------
    fig, ax = plt.subplots(figsize=(11, 6))
    ax.scatter(df['x'], df['emp'], s=15, alpha=0.3, label='Эмпирическая плотность', color='steelblue')
    ax.plot(df['x'], df['true_pdf'],   linewidth=2.5, label='Истинное распределение', color='black')
    ax.plot(df['x'], df['nonrob_pdf'], linewidth=2,   label='Неробастная аппр.',      color='darkorange',  linestyle='--')
    ax.plot(df['x'], df['rob_pdf'],    linewidth=2,   label='Робастная аппр.',         color='forestgreen', linestyle='-.')

    ax.set_title(f"{display_title}\nОбщее сравнение", fontsize=11, pad=10)
    ax.set_xlabel("x")
    ax.set_ylabel("Плотность")
    ax.legend()
    ax.grid(True, linestyle=':', alpha=0.7)
    fig.tight_layout()
    fig.savefig(os.path.join(output_dir, f"{file_stem}_general.png"), dpi=150)
    plt.close(fig)

    # ----------------------------------------------------------------
    # 2. ДЕТАЛИЗАЦИЯ: НЕРОБАСТНЫЙ МЕТОД
    # ----------------------------------------------------------------
    nr_cols = [c for c in df.columns if c.startswith('nr_c')]
    num_nr = len(nr_cols) + 1

    fig, axes = plt.subplots(num_nr, 1, figsize=(11, 3 * num_nr), sharex=True)
    if num_nr == 1:
        axes = [axes]

    fig.suptitle(f"{display_title}\nНеробастная модель — разложение по компонентам",
                 fontsize=11, y=1.01)

    axes[0].scatter(df['x'], df['emp'], s=10, alpha=0.3, color='steelblue', label='Эмпирика')
    axes[0].plot(df['x'], df['nonrob_pdf'], linewidth=2.5, color='darkorange',
                 label='Суммарная неробастная модель')
    axes[0].legend(loc="upper right", fontsize=8)
    axes[0].set_ylabel("Плотность")
    axes[0].grid(True, linestyle=':', alpha=0.7)

    for i, col in enumerate(nr_cols):
        ax = axes[i + 1]
        c_color = comp_colors[i % len(comp_colors)]
        ax.fill_between(df['x'], df['emp'], alpha=0.1, color='steelblue')
        ax.plot(df['x'], df[col], linewidth=2.5, color=c_color,
                label=f"Компонента {i + 1} (Нормальная)")
        ax.legend(loc="upper right", fontsize=8)
        ax.set_ylabel("Плотность")
        ax.grid(True, linestyle=':', alpha=0.7)

    axes[-1].set_xlabel("x")
    fig.tight_layout()
    fig.savefig(os.path.join(output_dir, f"{file_stem}_nonrobust_components.png"), dpi=150)
    plt.close(fig)

    # ----------------------------------------------------------------
    # 3. ДЕТАЛИЗАЦИЯ: РОБАСТНЫЙ МЕТОД
    # ----------------------------------------------------------------
    r_cols = [c for c in df.columns if c.startswith('r_c')]
    num_r = len(r_cols) + 1

    fig, axes = plt.subplots(num_r, 1, figsize=(11, 3 * num_r), sharex=True)
    if num_r == 1:
        axes = [axes]

    fig.suptitle(f"{display_title}\nРобастная модель — разложение по компонентам",
                 fontsize=11, y=1.01)

    axes[0].scatter(df['x'], df['emp'], s=10, alpha=0.3, color='steelblue', label='Эмпирика')
    axes[0].plot(df['x'], df['rob_pdf'], linewidth=2.5, color='forestgreen',
                 label='Суммарная робастная модель')
    axes[0].legend(loc="upper right", fontsize=8)
    axes[0].set_ylabel("Плотность")
    axes[0].grid(True, linestyle=':', alpha=0.7)

    for i, col in enumerate(r_cols):
        ax = axes[i + 1]
        is_noise = (i == len(r_cols) - 1)
        c_color = 'crimson' if is_noise else comp_colors[i % len(comp_colors)]
        lbl = "Равномерный шум (робастная добавка)" if is_noise else f"Компонента {i + 1} (Нормальная)"
        ax.fill_between(df['x'], df['emp'], alpha=0.1, color='steelblue')
        ax.plot(df['x'], df[col], linewidth=2.5, color=c_color, label=lbl)
        ax.legend(loc="upper right", fontsize=8)
        ax.set_ylabel("Плотность")
        ax.grid(True, linestyle=':', alpha=0.7)

    axes[-1].set_xlabel("x")
    fig.tight_layout()
    fig.savefig(os.path.join(output_dir, f"{file_stem}_robust_components.png"), dpi=150)
    plt.close(fig)

print(f"\nПостроение графиков завершено! Сохранено в: {output_dir}")