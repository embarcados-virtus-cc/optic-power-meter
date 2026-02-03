import { useEffect, useState } from 'react'
import { Activity, AlarmClock, Toolbox, Users } from 'lucide-react'
import { Link, useLocation } from '@tanstack/react-router'

import {
  NavigationMenu,
  NavigationMenuItem,
  NavigationMenuList,
} from '@/components/ui/navigation-menu'
import { Skeleton } from '@/components/ui/skeleton'
import { About } from '@/components/about/page/Sidebar'
import { ThemeToggle } from './ThemeToggle'

import { useStore } from '@tanstack/react-store'
import { themeStore } from '@/stores/themeStore'

export function Header() {
  const [isLoading, setIsLoading] = useState(true)
  const [isAboutOpen, setIsAboutOpen] = useState(false)
  const location = useLocation()
  const theme = useStore(themeStore)

  useEffect(() => {
    const timer = setTimeout(() => {
      setIsLoading(false)
    }, 500)

    return () => clearTimeout(timer)
  }, [])

  const isActive = (path: string) => {
    return location.pathname === path
  }

  return (
    <header className="p-5 flex items-center bg-background shadow-lg border-border border-b-[0.5px]">
      <div className="container mx-auto">
        <div className="flex items-center justify-between">
          {/* Logo do Projeto (alinhado a esquerda) */}
          <div className="flex items-center gap-2">
            {isLoading ? (
              <Skeleton className="h-12 w-95 bg-muted" />
            ) : (
              <Link to="/">
                <span className="text-xl font-bold text-foreground cursor-pointer flex items-center">
                  <img
                    src={
                      theme === 'light'
                        ? '/logo-power-meter-light.png'
                        : '/logo-power-meter.png'
                    }
                    alt="Logo"
                    className="h-12 w-12 mr-4"
                  />
                  MEDIDOR DE POTÊNCIA ÓPTICA
                  <img
                    src="/logo-virtus-cc.png"
                    alt="Logo Virtus CC"
                    className="h-16 w-60 ml-4"
                  />
                </span>
              </Link>
            )}
          </div>

          {/* Navbar Completa (alinhada a direita) */}
          {isLoading ? (
            // Skeleton da Navbar
            <div className="ml-auto flex gap-2">
              <Skeleton className="h-9 w-46 bg-muted" />
              <Skeleton className="h-9 w-49 bg-muted" />
              <Skeleton className="h-9 w-38 bg-muted" />
              <Skeleton className="h-9 w-24 bg-muted" />
              <Skeleton className="h-9 w-9 bg-muted" />
            </div>
          ) : (
            <NavigationMenu className="ml-auto">
              <NavigationMenuList className="gap-2">
                <NavigationMenuItem>
                  <Link
                    to="/"
                    className={`
                      flex flex-row items-center justify-start gap-2
                      text-left cursor-pointer
                      ${isActive('/') ? 'text-muted-foreground' : 'text-foreground'}
                      transition-colors
                      px-3 py-2 rounded-md
                    `}
                  >
                    <Activity
                      className={
                        isActive('/') ? 'text-muted-foreground' : 'text-foreground'
                      }
                      size={20}
                    />
                    Monitoramento
                  </Link>
                </NavigationMenuItem>

                <NavigationMenuItem>
                  <Link
                    to="/alarms"
                    className={`
                      flex flex-row items-center justify-start gap-2
                      text-left cursor-pointer
                      ${isActive('/alarms') ? 'text-muted-foreground' : 'text-foreground'}
                      transition-colors
                      px-3 py-2 rounded-md
                    `}
                  >
                    <AlarmClock
                      className={
                        isActive('/alarms') ? 'text-muted-foreground' : 'text-foreground'
                      }
                      size={20}
                    />
                    Alarmes e Avisos
                  </Link>
                </NavigationMenuItem>

                <NavigationMenuItem>
                  <Link
                    to="/tuning"
                    className={`
                      flex flex-row items-center justify-start gap-2
                      text-left cursor-pointer
                      ${isActive('/tuning') ? 'text-muted-foreground' : 'text-foreground'}
                      transition-colors
                      px-3 py-2 rounded-md
                    `}
                  >
                    <Toolbox
                      className={
                        isActive('/tuning') ? 'text-muted-foreground' : 'text-foreground'
                      }
                      size={20}
                    />
                    Calibração
                  </Link>
                </NavigationMenuItem>

                <NavigationMenuItem>
                  <button
                    onClick={() => setIsAboutOpen(true)}
                    className="
                      flex flex-row items-center justify-start gap-2
                      text-left cursor-pointer
                      text-foreground
                      transition-colors
                      px-3 py-2 rounded-md
                    "
                  >
                    <Users className="text-foreground" size={20} />
                    Sobre
                  </button>
                </NavigationMenuItem>

                <NavigationMenuItem>
                  <ThemeToggle />
                </NavigationMenuItem>
              </NavigationMenuList>
            </NavigationMenu>
          )}
        </div>
      </div>
      <About open={isAboutOpen} onOpenChange={setIsAboutOpen} />
    </header>
  )
}

export default Header
