import { useStore } from '@tanstack/react-store'
import { FlagTriangleRight, Trash2, TriangleAlert } from 'lucide-react'
import { Skeleton } from '../../ui/skeleton'
import { Label } from '../../ui/label'
import {
  CardComponent,
  CardContentComponent,
  CardHeaderComponent,
} from '@/components/ui/card'
import { cn } from '@/lib/utils'
import { alarmStore, removeAlarm } from '@/stores/alarmStore'
import type { AlarmFlag } from '@/stores/alarmStore'

// Re-exporting AlarmFlag for compatibility if needed, but better to import from store
export type { AlarmFlag }

export function Flags({ isLoading }: { isLoading: boolean }) {
  const flags = useStore(alarmStore)

  const handleDelete = (id: string) => {
    removeAlarm(id)
  }

  const getTypeStyles = (type: 'ALTO' | 'BAIXO') => {
    return type === 'ALTO'
      ? 'bg-red-900 text-white'
      : 'bg-yellow-500 text-zinc-900'
  }

  return (
    <CardComponent className="relative overflow-hidden w-full flex flex-col h-full">
      <CardHeaderComponent className="pb-8">
        <div className="flex items-center gap-3">
          {isLoading ? (
            <>
              <Skeleton className="w-7 h-7 bg-muted rounded-md" />
              <Skeleton className="w-72 h-7 bg-muted rounded-md" />
            </>
          ) : (
            <>
              <FlagTriangleRight className="text-foreground" size={28} />
              <Label className="text-xl font-bold uppercase tracking-wider text-foreground">
                Flags Ativas e Avisos Recentes
              </Label>
            </>
          )}
        </div>
      </CardHeaderComponent>

      <CardContentComponent className="flex-1">
        {isLoading ? (
          <div className="bg-secondary/50 rounded-lg border border-border p-3">
            <div className="flex flex-col gap-2">
              {Array.from({ length: 3 }).map((_, i) => (
                <div
                  key={i}
                  className="flex items-center gap-3 p-4 bg-secondary/50 rounded-lg border border-border"
                >
                  <Skeleton className="w-10 h-10 rounded-md bg-muted flex-shrink-0" />
                  <div className="flex-1 flex flex-col gap-1 min-w-0">
                    <Skeleton className="w-28 h-5 bg-muted rounded" />
                    <Skeleton className="w-16 h-3 bg-muted rounded" />
                  </div>
                  <Skeleton className="w-14 h-6 bg-muted rounded flex-shrink-0" />
                  <Skeleton className="w-12 h-6 bg-muted rounded flex-shrink-0" />
                  <Skeleton className="w-8 h-8 bg-muted rounded flex-shrink-0" />
                </div>
              ))}
            </div>
          </div>
        ) : flags.length === 0 ? (
          <div className="bg-secondary/50 rounded-lg border border-border p-6">
            <div className="flex flex-col items-center justify-center gap-4 text-muted-foreground py-8">
              <FlagTriangleRight size={40} className="opacity-30" />
              <p className="text-sm font-medium">
                Nenhuma flag ativa no momento
              </p>
            </div>
          </div>
        ) : (
          <div className="bg-secondary/50 rounded-lg border border-border p-3 max-h-[440px] overflow-y-auto custom-scrollbar">
            <div className="flex flex-col gap-2">
              {flags.map((flag) => (
                <div
                  key={flag.id}
                  className={cn(
                    'flex flex-col sm:flex-row sm:items-center gap-3 p-4 bg-secondary/50 rounded-lg border border-border hover:border-muted-foreground transition-all',
                  )}
                >
                  <div className="flex items-center gap-3 w-full sm:w-auto">
                    {/* Ícone de Warning */}
                    <div
                      className={cn(
                        'flex items-center justify-center w-10 h-10 rounded-md flex-shrink-0',
                        getTypeStyles(flag.type),
                      )}
                    >
                      <TriangleAlert size={20} />
                    </div>

                    {/* Nome do parâmetro e timestamp (Mobile) */}
                    <div className="flex-1 min-w-0 sm:hidden">
                      <div className="flex flex-col gap-0.5">
                        <span className="text-base font-bold text-foreground truncate">
                          {flag.parameter}
                        </span>
                        <span className="text-xs text-muted-foreground font-mono">
                          {flag.timestamp}
                        </span>
                      </div>
                    </div>

                    {/* Botão de deletar (Mobile) */}
                    <button
                      onClick={() => handleDelete(flag.id)}
                      className="flex sm:hidden items-center justify-center w-8 h-8 rounded hover:bg-zinc-800 transition-colors text-muted-foreground hover:text-red-400 flex-shrink-0 ml-auto"
                      aria-label="Remover flag"
                    >
                      <Trash2 size={16} />
                    </button>
                  </div>

                  {/* Desktop Content & Common Values */}
                  <div className="flex items-center gap-3 w-full">
                    {/* Nome e Timestamp (Desktop) */}
                    <div className="flex-1 min-w-0 hidden sm:block">
                      <div className="flex flex-col gap-0.5">
                        <span className="text-base font-bold text-foreground truncate">
                          {flag.parameter}
                        </span>
                        <span className="text-xs text-muted-foreground font-mono">
                          {flag.timestamp}
                        </span>
                      </div>
                    </div>

                    <div className="flex items-center gap-3 ml-auto sm:ml-0">
                      {/* Valor */}
                      <div className="text-base font-bold text-foreground flex-shrink-0">
                        {typeof flag.value === 'number'
                          ? flag.value.toFixed(2)
                          : flag.value}
                        °
                      </div>

                      {/* Badge do tipo */}
                      <div
                        className={cn(
                          'px-2 py-1 rounded text-xs font-black uppercase flex-shrink-0',
                          getTypeStyles(flag.type),
                        )}
                      >
                        {flag.type}
                      </div>

                      {/* Botão de deletar (Desktop) */}
                      <button
                        onClick={() => handleDelete(flag.id)}
                        className="hidden sm:flex items-center justify-center w-8 h-8 rounded hover:bg-zinc-800 transition-colors text-muted-foreground hover:text-red-400 flex-shrink-0"
                        aria-label="Remover flag"
                      >
                        <Trash2 size={16} />
                      </button>
                    </div>
                  </div>
                </div>
              ))}
            </div>
          </div>
        )}
      </CardContentComponent>
    </CardComponent>
  )
}
