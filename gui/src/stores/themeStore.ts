import { Store } from '@tanstack/store'

export type Theme = 'light' | 'dark'

const getInitialTheme = (): Theme => {
    if (typeof window === 'undefined') return 'dark'
    try {
        const stored = localStorage.getItem('theme') as Theme | null
        return stored || 'dark'
    } catch {
        return 'dark'
    }
}

export const themeStore = new Store<Theme>(getInitialTheme())

export const toggleTheme = () => {
    themeStore.setState((current) => {
        const next = current === 'dark' ? 'light' : 'dark'
        applyTheme(next)
        return next
    })
}

export const setTheme = (theme: Theme) => {
    themeStore.setState(() => {
        applyTheme(theme)
        return theme
    })
}

const applyTheme = (theme: Theme) => {
    if (typeof window === 'undefined') return

    try {
        localStorage.setItem('theme', theme)
    } catch {
        // localStorage not available (e.g., test environment)
    }

    if (theme === 'dark') {
        document.documentElement.classList.add('dark')
    } else {
        document.documentElement.classList.remove('dark')
    }
}

// Apply initial theme on load
if (typeof window !== 'undefined') {
    applyTheme(themeStore.state)
}
