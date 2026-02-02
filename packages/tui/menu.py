from prompt_toolkit import Application
from prompt_toolkit.key_binding import KeyBindings
from prompt_toolkit.layout import Layout, HSplit, VSplit, Window
from prompt_toolkit.layout.controls import FormattedTextControl
from prompt_toolkit.styles import Style

# ===========================
# ASCII do Título
# ===========================
TITLE = """
 ██████╗ ██████╗ ████████╗██╗ ██████╗    ██████╗  ██████╗ ██╗    ██╗███████╗██████╗    ███╗   ███╗███████╗████████╗███████╗██████╗
██╔═══██╗██╔══██╗╚══██╔══╝██║██╔════╝    ██╔══██╗██╔═══██╗██║    ██║██╔════╝██╔══██╗   ████╗ ████║██╔════╝╚══██╔══╝██╔════╝██╔══██╗
██║   ██║██████╔╝   ██║   ██║██║         ██████╔╝██║   ██║██║ █╗ ██║█████╗  ██████╔╝   ██╔████╔██║█████╗     ██║   █████╗  ██████╔╝
██║   ██║██╔═══╝    ██║   ██║██║         ██╔═══╝ ██║   ██║██║███╗██║██╔══╝  ██╔══██╗   ██║╚██╔╝██║██╔══╝     ██║   ██╔══╝  ██╔══██╗
╚██████╔╝██║        ██║   ██║╚██████╗    ██║     ╚██████╔╝╚███╔███╔╝███████╗██║  ██║   ██║ ╚═╝ ██║███████╗   ██║   ███████╗██║  ██║
 ╚═════╝ ╚═╝        ╚═╝   ╚═╝ ╚═════╝    ╚═╝      ╚═════╝  ╚══╝╚══╝ ╚══════╝╚═╝  ╚═╝   ╚═╝     ╚═╝╚══════╝   ╚═╝   ╚══════╝╚═╝  ╚═╝
"""

# ===========================
# Estado
# ===========================
MENU_ITEMS = ["Configurações do Transceptor", "Valores Atuais de Potência Óptica", "Informações do Transceptor", "Sair"]
selected_index = 0

# ===========================
# Menu
# ===========================
def menu_text():
    result = []
    for i, item in enumerate(MENU_ITEMS):
        if i == selected_index:
            result.append(("class:arrow", "▶ "))
            result.append(("class:item", item + "\n"))
        else:
            result.append(("class:item", "  " + item + "\n"))
    return result

# ===========================
# Conteúdo
# ===========================
def content_text():
    # ===========================
    # Helper para formatar janelas
    # ===========================
    def format_content(title, body):
        return f"\n{title}\n\n{body}\n"

    page = MENU_ITEMS[selected_index]
    if page == "Configurações do Transceptor":
        return format_content(
            "CONFIGURAÇÕES DO TRANSCEPTOR",
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        )
    if page == "Valores Atuais de Potência Óptica":
        return format_content(
            "VALORES ATUAIS DE POTÊNCIA ÓPTICA",
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        )
    if page == "Informações do Transceptor":
        return format_content(
            "INFORMAÇÕES DO TRANSCEPTOR",
            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        )
    return ""

# ===========================
# Componentes
# ===========================
title_window = Window(
    content=FormattedTextControl(
        lambda: [("class:title", TITLE)]
    ),
    height=8,
    always_hide_cursor=True,
)

horizontal_line = Window(
    char="═",
    height=1,
    style="class:border",
)

vertical_line = Window(
    char="║",
    width=1,
    style="class:border",
)

padding_horizontal = Window(width=2)
padding_vertical = Window(height=1)

menu_window = HSplit([
    padding_vertical,
    Window(
        content=FormattedTextControl(menu_text),
        width=38,
        always_hide_cursor=True,
    ),
])

content_window = Window(
    content=FormattedTextControl(content_text),
    always_hide_cursor=True,
)

# ===========================
# Layout
# ===========================
layout = Layout(
    HSplit(
        [
            title_window,
            horizontal_line,
            VSplit(
                [
                    padding_horizontal,
                    menu_window,
                    padding_horizontal,
                    vertical_line,
                    padding_horizontal,
                    content_window,
                ]
            ),
            horizontal_line,
        ]
    )
)

# ===========================
# Teclas
# ===========================
kb = KeyBindings()

@kb.add("up")
def _(event):
    global selected_index
    selected_index = (selected_index - 1) % len(MENU_ITEMS)

@kb.add("down")
def _(event):
    global selected_index
    selected_index = (selected_index + 1) % len(MENU_ITEMS)

@kb.add("enter")
def _(event):
    if MENU_ITEMS[selected_index] == "Sair":
        event.app.exit()

@kb.add("q")
@kb.add("escape")
def _(event):
    event.app.exit()

# ===========================
# Estilo
# ===========================
style = Style.from_dict(
    {
        "title": "bold fg:#ffffff",
        "arrow": "fg:#c084fc bold",
        "item": "",
        "border": "fg:#ffffff",
    }
)

# ===========================
# App
# ===========================
app = Application(
    layout=layout,
    key_bindings=kb,
    style=style,
    full_screen=True,
    cursor=None,
)

if __name__ == "__main__":
    app.run()
