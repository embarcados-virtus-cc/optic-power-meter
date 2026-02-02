from prompt_toolkit import Application
from prompt_toolkit.key_binding import KeyBindings
from prompt_toolkit.layout import Layout, HSplit, VSplit, Window
from prompt_toolkit.layout.controls import FormattedTextControl
from prompt_toolkit.styles import Style
from prompt_toolkit.application import get_app
import threading
import time
from daemon_client import DaemonClient
from formatters import format_flags_only, format_dynamic_values_only, format_all_a0h_fields, format_flags_template, format_dynamic_template, format_all_a0h_template

# ===========================
# ASCII do Título + Logo Virtus CC
# ===========================
TITLE_LINES = [
    " ██████╗ ██████╗ ████████╗██╗ ██████╗    ██████╗  ██████╗ ██╗    ██╗███████╗██████╗    ███╗   ███╗███████╗████████╗███████╗██████╗ ",
    "██╔═══██╗██╔══██╗╚══██╔══╝██║██╔════╝    ██╔══██╗██╔═══██╗██║    ██║██╔════╝██╔══██╗   ████╗ ████║██╔════╝╚══██╔══╝██╔════╝██╔══██╗",
    "██║   ██║██████╔╝   ██║   ██║██║         ██████╔╝██║   ██║██║ █╗ ██║█████╗  ██████╔╝   ██╔████╔██║█████╗     ██║   █████╗  ██████╔╝",
    "██║   ██║██╔═══╝    ██║   ██║██║         ██╔═══╝ ██║   ██║██║███╗██║██╔══╝  ██╔══██╗   ██║╚██╔╝██║██╔══╝     ██║   ██╔══╝  ██╔══██╗",
    "╚██████╔╝██║        ██║   ██║╚██████╗    ██║     ╚██████╔╝╚███╔███╔╝███████╗██║  ██║   ██║ ╚═╝ ██║███████╗   ██║   ███████╗██║  ██║",
    " ╚═════╝ ╚═╝        ╚═╝   ╚═╝ ╚═════╝    ╚═╝      ╚═════╝  ╚══╝╚══╝ ╚══════╝╚═╝  ╚═╝   ╚═╝     ╚═╝╚══════╝   ╚═╝   ╚══════╝╚═╝  ╚═╝",
]

# Logo Virtus CC em ASCII art com cores
VIRTUS_LOGO_LINES = [
    "██╗   ██╗██╗██████╗ ████████╗██╗   ██╗███████╗     ██████╗ ██████╗ ",
    "██║   ██║██║██╔══██╗╚══██╔══╝██║   ██║██╔════╝    ██╔════╝██╔════╝ ",
    "██║   ██║██║██████╔╝   ██║   ██║   ██║███████╗    ██║     ██║      ",
    "╚██╗ ██╔╝██║██╔══██╗   ██║   ██║   ██║╚════██║    ██║     ██║      ",
    " ╚████╔╝ ██║██║  ██║   ██║   ╚██████╔╝███████║    ╚██████╗╚██████╗ ",
    "  ╚═══╝  ╚═╝╚═╝  ╚═╝   ╚═╝    ╚═════╝ ╚══════╝     ╚═════╝ ╚═════╝ ",
]

def get_title_formatted():
    """Retorna título e logo Virtus CC lado a lado com cores"""
    result = []
    result.append(("", "\n"))
    
    # Largura da primeira linha do título para padding
    title_width = len(TITLE_LINES[0])
    
    for i in range(max(len(TITLE_LINES), len(VIRTUS_LOGO_LINES))):
        # 1. Adiciona linha do título principal (ou espaços se acabou)
        if i < len(TITLE_LINES):
            result.append(("class:title", TITLE_LINES[i]))
        else:
            result.append(("", " " * title_width))
            
        # 2. Espaçador entre título e logo
        result.append(("", "    "))
        
        # 3. Adiciona linha do logo com cores específicas
        if i < len(VIRTUS_LOGO_LINES):
            line = VIRTUS_LOGO_LINES[i]
            if i == 0:
                result.append(("class:logo-magenta", "  ║"))
                result.append(("class:logo-purple", "║"))
                result.append(("class:logo-blue", "║"))
                result.append(("class:logo-cyan", "║"))
                result.append(("class:logo-teal", "║"))
                result.append(("", line[7:] + "\n"))
            elif i == 1:
                result.append(("class:logo-magenta", "  ║"))
                result.append(("class:logo-purple", "║"))
                result.append(("class:logo-blue", "║"))
                result.append(("class:logo-cyan", "║"))
                result.append(("class:logo-teal", "║  "))
                result.append(("class:logo-virtus", "██╗   ██╗██╗██████╗ ████████╗██╗   ██╗███████╗"))
                result.append(("class:logo-cc", "     ██████╗ ██████╗ \n"))
            elif i == 2:
                result.append(("class:logo-magenta", " ╔╝"))
                result.append(("class:logo-purple", "║"))
                result.append(("class:logo-blue", "║"))
                result.append(("class:logo-cyan", "║"))
                result.append(("class:logo-teal", "║  "))
                result.append(("class:logo-virtus", "██║   ██║██║██╔══██╗╚══██╔══╝██║   ██║██╔════╝"))
                result.append(("class:logo-cc", "    ██╔════╝██╔════╝ \n"))
            elif i == 3:
                result.append(("class:logo-magenta", " ║ "))
                result.append(("class:logo-purple", "║"))
                result.append(("class:logo-blue", "║"))
                result.append(("class:logo-cyan", "║"))
                result.append(("class:logo-teal", "║  "))
                result.append(("class:logo-virtus", "██║   ██║██║██████╔╝   ██║   ██║   ██║███████╗"))
                result.append(("class:logo-cc", "    ██║     ██║      \n"))
            elif i == 4:
                result.append(("class:logo-magenta", " ║ "))
                result.append(("class:logo-purple", "║"))
                result.append(("class:logo-blue", "╚╝"))
                result.append(("class:logo-cyan", "║  "))
                result.append(("class:logo-virtus", "╚██╗ ██╔╝██║██╔══██╗   ██║   ██║   ██║╚════██║"))
                result.append(("class:logo-cc", "    ██║     ██║      \n"))
            elif i == 5:
                result.append(("class:logo-magenta", " ╚╗"))
                result.append(("class:logo-purple", "║  "))
                result.append(("class:logo-cyan", "╚╗ "))
                result.append(("class:logo-virtus", " ╚████╔╝ ██║██║  ██║   ██║   ╚██████╔╝███████║"))
                result.append(("class:logo-cc", "    ╚██████╗╚██████╗ \n"))
            elif i == 6:
                result.append(("class:logo-magenta", "  ╚╝"))
                result.append(("class:logo-cyan", "  ╚╝  "))
                result.append(("class:logo-virtus", " ╚═══╝  ╚═╝╚═╝  ╚═╝   ╚═╝    ╚═════╝ ╚══════╝"))
                result.append(("class:logo-cc", "     ╚═════╝ ╚═════╝ \n"))
        else:
            result.append(("", "\n"))
    
    return result

# ===========================
# Estado
# ===========================
MENU_ITEMS = ["Configurações do Transceptor", "Valores Atuais de Potência Óptica", "Informações do Transceptor", "Sair"]
selected_index = 0
debug_mode = False  # Modo debug para ver templates
scroll_offset = 0  # Offset de scroll vertical

# Cache de dados
cached_data = {
    "current": None,
    "static": None,
    "dynamic": None,
    "last_update": 0,
    "error": None
}

# Cliente daemon
client = DaemonClient()

# Lock para thread-safety
data_lock = threading.Lock()

# Intervalo de atualização (segundos)
UPDATE_INTERVAL = 2.0


# ===========================
# Atualização de Dados
# ===========================
def update_data():
    """Atualiza dados do daemon em background"""
    global cached_data

    while True:
        try:
            # Tenta obter dados completos
            try:
                current_data = client.get_current()
                with data_lock:
                    cached_data["current"] = current_data
                    cached_data["static"] = current_data  # GET CURRENT inclui A0h
                    cached_data["dynamic"] = current_data  # GET CURRENT inclui A2h
                    cached_data["last_update"] = time.time()
                    cached_data["error"] = None
            except FileNotFoundError:
                with data_lock:
                    cached_data["error"] = "SFP não encontrado"
                    cached_data["current"] = None
                    cached_data["static"] = None
                    cached_data["dynamic"] = None
            except (ConnectionError, TimeoutError) as e:
                with data_lock:
                    cached_data["error"] = f"Erro de conexão: {str(e)}"
                    cached_data["current"] = None
                    cached_data["static"] = None
                    cached_data["dynamic"] = None
            except Exception as e:
                with data_lock:
                    cached_data["error"] = f"Erro: {str(e)}"
                    cached_data["current"] = None
                    cached_data["static"] = None
                    cached_data["dynamic"] = None

            # Atualiza UI (tenta, mas não falha se app não estiver pronto)
            try:
                app = get_app()
                if app:
                    app.invalidate()
            except Exception:
                pass  # App ainda não está pronto

        except Exception:
            pass  # Ignora erros no thread de atualização

        time.sleep(UPDATE_INTERVAL)


# Inicia thread de atualização
update_thread = threading.Thread(target=update_data, daemon=True)
update_thread.start()


# ===========================
# Menu
# ===========================
def menu_text():
    result = []
    for i, item in enumerate(MENU_ITEMS):
        if i == selected_index:
            result.append(("class:arrow", "▶ "))
            result.append(("class:item-selected", item + "\n"))  # Item selecionado com cor roxa
        else:
            result.append(("class:item", "  " + item + "\n"))
    return result


# ===========================
# Conteúdo (com scroll)
# ===========================
def get_full_content():
    """Obtém todo o conteúdo sem scroll"""
    global debug_mode
    page = MENU_ITEMS[selected_index]

    # Modo debug: mostra templates sem dados
    if debug_mode:
        try:
            if page == "Configurações do Transceptor":
                return format_flags_template()
            elif page == "Valores Atuais de Potência Óptica":
                return format_dynamic_template()
            elif page == "Informações do Transceptor":
                return format_all_a0h_template()
            else:
                return [("", "")]
        except Exception as e:
            return [
                ("class:error", f"\nERRO\n\n"),
                ("class:error", f"Erro ao formatar template: {str(e)}\n")
            ]

    with data_lock:
        error = cached_data.get("error")
        last_update = cached_data.get("last_update", 0)

    # Mensagem de erro geral
    if error:
        result = [
            ("class:error", f"\nERRO\n\n"),
            ("class:error", f"{error}\n\n"),
            ("class:label", "Verifique se o daemon está rodando e se o SFP está conectado.\n")
        ]
        return result

    # Mensagem de carregamento
    if last_update == 0:
        result = [
            ("class:warning", "\nCarregando dados...\n\n"),
            ("class:label", "Aguardando conexão com o daemon...\n")
        ]
        return result

    # Conteúdo específico por página
    try:
        with data_lock:
            if page == "Configurações do Transceptor":
                data = cached_data.get("current")
                if not data:
                    return [("class:warning", "\nNenhum dado disponível\n")]
                formatted = format_flags_only(data)
                return formatted

            elif page == "Valores Atuais de Potência Óptica":
                data = cached_data.get("current")
                if not data:
                    return [("class:warning", "\nNenhum dado disponível\n")]
                formatted = format_dynamic_values_only(data)
                return formatted

            elif page == "Informações do Transceptor":
                data = cached_data.get("current")
                if not data:
                    return [("class:warning", "\nNenhum dado disponível\n")]
                formatted = format_all_a0h_fields(data)
                return formatted

            else:
                return [("", "")]

    except Exception as e:
        return [
            ("class:error", f"\nERRO\n\n"),
            ("class:error", f"Erro ao formatar dados: {str(e)}\n")
        ]


def content_text():
    """Conteúdo com scroll aplicado e indicadores de scroll"""
    global scroll_offset

    # Obtém todo o conteúdo
    full_content = get_full_content()

    # Converte para lista de linhas (cada linha é uma lista de tuplas (style, text))
    lines = []
    current_line = []
    current_text = ""
    current_style = None

    for style, text in full_content:
        if not text:
            continue

        for char in text:
            if char == '\n':
                # Fim de linha
                if current_text:
                    current_line.append((current_style or "class:value", current_text))
                if current_line:
                    lines.append(current_line)
                current_line = []
                current_text = ""
                current_style = None
            else:
                if style != current_style:
                    if current_text:
                        current_line.append((current_style or "class:value", current_text))
                    current_style = style
                    current_text = char
                else:
                    current_text += char

    # Adiciona última linha se não terminou com \n
    if current_text:
        current_line.append((current_style or "class:value", current_text))
    if current_line:
        lines.append(current_line)

    # Aplica scroll
    total_lines = len(lines)
    try:
        from prompt_toolkit.output import get_default_output
        output = get_default_output()
        if hasattr(output, 'get_size'):
            size = output.get_size()
            # Altura disponível: total - título (8) - linhas (2) - status (1) - help (1) - indicadores (2) = 14
            visible_height = size.rows - 14
            visible_height = max(5, visible_height)  # Mínimo de 5 linhas
        else:
            visible_height = 34  # 36 - 2 para os indicadores
    except:
        visible_height = 34

    # Limita scroll_offset
    max_offset = max(0, total_lines - visible_height)
    scroll_offset = max(0, min(scroll_offset, max_offset))

    # Calcula se há conteúdo acima ou abaixo
    has_content_above = scroll_offset > 0
    has_content_below = scroll_offset < max_offset

    # Retorna apenas linhas visíveis
    visible_lines = lines[scroll_offset:scroll_offset + visible_height]

    # Reconstrói formato FormattedText
    result = []

    # Indicador de scroll superior (se há conteúdo acima)
    if has_content_above:
        result.append(("class:scroll-indicator", "                         ▲ Mais conteúdo acima (tecla H) ▲\n"))
    else:
        result.append(("", "\n"))  # Linha em branco para manter espaçamento

    for line in visible_lines:
        result.extend(line)
        result.append(("", "\n"))

    # Indicador de scroll inferior (se há conteúdo abaixo)
    if has_content_below:
        result.append(("class:scroll-indicator", "                         ▼ Mais conteúdo abaixo (tecla L) ▼"))
    else:
        result.append(("", ""))  # Linha em branco para manter espaçamento

    return result if result else [("", "")]


# ===========================
# Status Bar
# ===========================
def status_text():
    global debug_mode

    if debug_mode:
        return [("class:warning", "DEBUG ATIVO - Templates sem dados")]

    with data_lock:
        last_update = cached_data.get("last_update", 0)
        error = cached_data.get("error")

    if error:
        return [("class:error", f"{error}")]

    if last_update > 0:
        elapsed = time.time() - last_update
        if elapsed < UPDATE_INTERVAL:
            return [("class:success", f" Atualizado há {elapsed:.1f}s")]
        else:
            return [("class:warning", f" Atualizando...")]

    return [("class:warning", " Conectando...")]


def help_text():
    """Linha de ajuda com todas as teclas disponíveis"""
    return [
        ("class:label", "Teclas: "),
        ("class:value", "↑/↓ = Navegar menu | "),
        ("class:value", "H = Scroll ▲ | "),
        ("class:value", "L = Scroll ▼ | "),
        ("class:value", "R = Atualizar | "),
        ("class:value", "P = [Debug] | "),
        ("class:value", "Q/ESC = Sair")
    ]


# ===========================
# Componentes
# ===========================
title_window = Window(
    content=FormattedTextControl(get_title_formatted),
    height=10,
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
    wrap_lines=True,
)

status_window = Window(
    content=FormattedTextControl(status_text),
    height=1,
    always_hide_cursor=True,
)

help_window = Window(
    content=FormattedTextControl(help_text),
    height=1,
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
            status_window,
            help_window,
        ]
    )
)

# ===========================
# Teclas
# ===========================
kb = KeyBindings()

@kb.add("up")
def _(event):
    global selected_index, scroll_offset
    old_index = selected_index
    selected_index = (selected_index - 1) % len(MENU_ITEMS)
    # Reseta scroll se mudou de página
    if old_index != selected_index:
        scroll_offset = 0
    event.app.invalidate()

@kb.add("down")
def _(event):
    global selected_index, scroll_offset
    old_index = selected_index
    selected_index = (selected_index + 1) % len(MENU_ITEMS)
    # Reseta scroll se mudou de página
    if old_index != selected_index:
        scroll_offset = 0
    event.app.invalidate()

@kb.add("enter")
def _(event):
    if MENU_ITEMS[selected_index] == "Sair":
        event.app.exit()

@kb.add("r")
def _(event):
    """Força atualização manual"""
    global cached_data
    with data_lock:
        cached_data["last_update"] = 0
    event.app.invalidate()

@kb.add("p")
def _(event):
    """ativa/desativa modo debug"""
    global debug_mode
    debug_mode = not debug_mode
    event.app.invalidate()

@kb.add("h")
def _(event):
    """Scroll para cima"""
    global scroll_offset
    scroll_offset = max(0, scroll_offset - 5)
    event.app.invalidate()

@kb.add("l")
def _(event):
    """Scroll para baixo"""
    global scroll_offset
    scroll_offset += 5
    event.app.invalidate()

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
        "item-selected": "fg:#c084fc bold",
        "border": "fg:#ffffff",
        "section": "fg:#ffffff bold",
        "label": "fg:#94a3b8",
        "value": "fg:#e2e8f0",
        "error": "fg:#ef4444 bold",
        "warning": "fg:#ffffff",
        "success": "fg:#10b981",
        "scroll-indicator": "fg:#c084fc bold",
        # Cores do logo Virtus CC
        "logo-magenta": "fg:#e91e8c bold",  # Magenta/rosa
        "logo-purple": "fg:#9333ea bold",   # Roxo
        "logo-blue": "fg:#3b82f6 bold",     # Azul
        "logo-cyan": "fg:#06b6d4 bold",     # Ciano
        "logo-teal": "fg:#14b8a6 bold",     # Teal/verde-água
        "logo-virtus": "fg:#1e3a5f bold",   # Azul escuro (VIRTUS)
        "logo-cc": "fg:#14b8a6 bold",       # Teal (CC)
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
    refresh_interval=0.5,  # Atualiza UI a cada 500ms
)

if __name__ == "__main__":
    app.run()
