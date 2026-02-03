import { useEffect, useState } from 'react'
import { Github } from 'lucide-react'
import { Skeleton } from '@/components/ui/skeleton'
import { Separator } from '@/components/ui/separator'
import { useStore } from '@tanstack/react-store'
import { themeStore } from '@/stores/themeStore'

export function Footer() {
  const [isLoading, setIsLoading] = useState(true)
  const theme = useStore(themeStore)

  useEffect(() => {
    const timer = setTimeout(() => {
      setIsLoading(false)
    }, 500)

    return () => clearTimeout(timer)
  }, [])

  return (
    <footer className="p-5 bg-background shadow-lg border-border border-t-[0.5px]">
      <div className="container mx-auto">
        {isLoading ? (
          // Skeleton do Footer
          <div className="flex flex-col gap-4">
            <div className="flex items-center justify-between">
              <div className="flex items-center gap-4">
                <div>
                  <Skeleton className="h-20 w-20 bg-muted" />
                </div>
                <div className="flex flex-col gap-2">
                  <Skeleton className="h-6 w-70 bg-muted" />
                  <Skeleton className="h-6 w-90 bg-muted" />
                </div>
              </div>
              <div className="flex gap-3">
                <Skeleton className="h-9 w-9 rounded-md bg-muted" />
              </div>
            </div>
            <Separator className="bg-border" />
          </div>
        ) : (
          <div className="flex flex-col gap-4">
            {/* Linha Principal */}
            <div className="flex flex-col md:flex-row items-center justify-between gap-4 md:gap-0">
              {/* Info do Projeto */}

              <div className="flex flex-col sm:flex-row items-center gap-4 sm:gap-2 text-center sm:text-left">
                <div className="sm:mr-2">
                  <img
                    src={
                      theme === 'light'
                        ? '/logo-power-meter-light.png'
                        : '/logo-power-meter.png'
                    }
                    alt="Logo do Medidor de Potência Óptica"
                    className="h-16 w-16 sm:h-20 sm:w-20 rounded-md"
                  />
                </div>
                <div className="flex flex-col gap-1 sm:gap-2">
                  <span className="text-base sm:text-lg font-bold text-foreground">
                    Medidor de Potência Óptica (2026)
                  </span>
                  <span className="text-xs sm:text-sm text-muted-foreground">
                    Sistema de Monitoramento de Potência Óptica
                  </span>
                </div>
                <div className="sm:ml-2">
                  <img
                    src="/logo-virtus-cc.png"
                    alt="Logo Virtus CC"
                    className="h-12 w-auto sm:h-20 sm:w-auto ml-0 sm:ml-4"
                  />
                </div>
              </div>

              {/* Links Sociais */}
              <div className="flex gap-3">
                <a
                  href="https://github.com/embarcados-virtus-cc/optic-power-meter"
                  target="_blank"
                  rel="noopener noreferrer"
                  className="
                    group
                    p-2 rounded-md
                    text-muted-foreground
                    bg-transparent
                    hover:bg-accent
                    hover:text-foreground
                    transition-colors
                    cursor-pointer
                  "
                  aria-label="GitHub"
                >
                  <Github
                    size={20}
                    className="
                      text-muted-foreground
                      group-hover:text-foreground
                      transition-colors
                    "
                  />
                </a>
              </div>
            </div>

            <Separator className="bg-border" />
          </div>
        )}
      </div>
    </footer>
  )
}

export default Footer
