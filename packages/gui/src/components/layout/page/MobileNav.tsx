import { useState } from 'react'
import { Link, useLocation } from '@tanstack/react-router'
import { Menu, Activity, AlarmClock, Toolbox, Users } from 'lucide-react'
import { Button } from '@/components/ui/button'
import {
    Sheet,
    SheetContent,
    SheetHeader,
    SheetTitle,
    SheetTrigger,
} from '@/components/ui/sheet'
import { ThemeToggle } from './ThemeToggle'
import { About } from '@/components/about/page/Sidebar'

export function MobileNav() {
    const [open, setOpen] = useState(false)
    const [isAboutOpen, setIsAboutOpen] = useState(false)
    const location = useLocation()

    const isActive = (path: string) => {
        return location.pathname === path
    }

    const handleLinkClick = () => {
        setOpen(false)
    }

    return (
        <>
            <Sheet open={open} onOpenChange={setOpen}>
                <SheetTrigger asChild>
                    <Button variant="ghost" size="icon" className="md:hidden">
                        <Menu className="h-6 w-6" />
                        <span className="sr-only">Menu de navegação</span>
                    </Button>
                </SheetTrigger>
                <SheetContent side="left" className="w-[300px] sm:w-[400px]">
                    <SheetHeader className="text-left mb-6">
                        <SheetTitle>Menu</SheetTitle>
                    </SheetHeader>
                    <nav className="flex flex-col gap-4">
                        <Link
                            to="/"
                            onClick={handleLinkClick}
                            className={`
                flex items-center gap-2 px-4 py-2 rounded-md transition-colors
                ${isActive('/')
                                    ? 'bg-secondary text-foreground font-medium'
                                    : 'text-muted-foreground hover:text-foreground hover:bg-secondary/50'
                                }
              `}
                        >
                            <Activity size={20} />
                            Monitoramento
                        </Link>

                        <Link
                            to="/alarms"
                            onClick={handleLinkClick}
                            className={`
                flex items-center gap-2 px-4 py-2 rounded-md transition-colors
                ${isActive('/alarms')
                                    ? 'bg-secondary text-foreground font-medium'
                                    : 'text-muted-foreground hover:text-foreground hover:bg-secondary/50'
                                }
              `}
                        >
                            <AlarmClock size={20} />
                            Alarmes e Avisos
                        </Link>

                        <Link
                            to="/tuning"
                            onClick={handleLinkClick}
                            className={`
                flex items-center gap-2 px-4 py-2 rounded-md transition-colors
                ${isActive('/tuning')
                                    ? 'bg-secondary text-foreground font-medium'
                                    : 'text-muted-foreground hover:text-foreground hover:bg-secondary/50'
                                }
              `}
                        >
                            <Toolbox size={20} />
                            Calibração
                        </Link>

                        <button
                            onClick={() => {
                                setOpen(false)
                                setIsAboutOpen(true)
                            }}
                            className="
                flex items-center gap-2 px-4 py-2 rounded-md transition-colors text-left
                text-muted-foreground hover:text-foreground hover:bg-secondary/50
              "
                        >
                            <Users size={20} />
                            Sobre
                        </button>

                        <div className="px-4 py-2">
                            <div className="flex items-center justify-between">
                                <span className="text-sm text-muted-foreground">Tema</span>
                                <ThemeToggle />
                            </div>
                        </div>
                    </nav>
                </SheetContent>
            </Sheet>

            <About open={isAboutOpen} onOpenChange={setIsAboutOpen} />
        </>
    )
}
