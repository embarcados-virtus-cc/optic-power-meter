from prompt_toolkit import Application
from prompt_toolkit.key_binding import KeyBindings
from prompt_toolkit.layout import Layout, HSplit, VSplit, Window
from prompt_toolkit.layout.controls import FormattedTextControl
from prompt_toolkit.styles import Style

# ===========================
# ASCII do Título
# ===========================
TITLE_ART = """
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
MENU_ITEMS = ["Configuração", "Valores Atuais", "Sair"]
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
    page = MENU_ITEMS[selected_index]
    if page == "Configuração":
        return "CONFIGURAÇÃO\n\nAqui você implementa depois.\n"
    if page == "Valores Atuais":
        return "VALORES ATUAIS\n\nAqui você implementa depois.\n"
    return ""

# ===========================
# Componentes
# ===========================
title_window = Window(
    content=FormattedTextControl(
        lambda: [("class:title", TITLE_ART)]
    ),
    height=8,
    always_hide_cursor=True,
)

horizontal_line = Window(
    char="─",
    height=1,
)

vertical_line = Window(
    char="│",
    width=1,
)

menu_window = Window(
    content=FormattedTextControl(menu_text),
    width=24,
    always_hide_cursor=True,
)

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
                    menu_window,
                    vertical_line,
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
        "arrow": "fg:#5fd7ff bold",
        "item": "",
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
