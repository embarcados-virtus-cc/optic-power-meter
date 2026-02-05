import { Moon, Sun } from 'lucide-react'
import { useStore } from '@tanstack/react-store'
import { themeStore, toggleTheme } from '@/stores/themeStore'

export function ThemeToggle() {
    const theme = useStore(themeStore)

    return (
        <button onClick={toggleTheme} className=" flex items-center justify-center p-2 rounded-md text-zinc-700 dark:text-slate-300 hover:bg-accent transition-colors cursor-pointer " aria-label={theme === 'dark' ? 'Ativar modo claro' : 'Ativar modo escuro'}>
            {theme === 'dark' ? <Sun size={20} /> : <Moon size={20} />}
        </button>
    )
}

export default ThemeToggle
