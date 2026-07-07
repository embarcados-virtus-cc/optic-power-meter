import { useEffect, useState } from 'react'
import { createFileRoute } from '@tanstack/react-router'
import { useStore } from '@tanstack/react-store'
import { RefreshCw, ServerCrash } from 'lucide-react'
import { Button } from '@/components/ui/button'
import { Skeleton } from '@/components/ui/skeleton'
import { ContainerCard } from '@/components/system/ContainerCard'
import { containersStore, containerStatusStore } from '@/stores/containerStore'
import { api, type ContainerStats } from '@/lib/api'

export const Route = createFileRoute('/system/')({
  component: SystemPage,
})

function SystemPage() {
  const containers = useStore(containersStore)
  const status = useStore(containerStatusStore)
  const [statsMap, setStatsMap] = useState<Record<string, ContainerStats>>({})

  useEffect(() => {
    const running = containers.filter((c) => c.status === 'running')
    if (!running.length) return
    let cancelled = false
    const fetchSequential = async () => {
      const map: Record<string, ContainerStats> = {}
      for (const c of running) {
        if (cancelled) break
        try {
          map[c.name] = await api.containerStats(c.name)
        } catch {
          // skip failed container
        }
      }
      if (!cancelled) setStatsMap(map)
    }
    void fetchSequential()
    return () => { cancelled = true }
  }, [containers])

  return (
    <div className="min-h-max bg-background">
      <main className="container mx-auto p-6">
        <div className="space-y-6">
          <div className="flex items-center justify-between">
            <div>
              <h1 className="text-2xl font-bold">Sistema</h1>
              <p className="text-sm text-muted-foreground">Gerenciamento de containers Docker</p>
            </div>
            <Button
              variant="outline"
              size="sm"
              onClick={() => api.containers().then((c) => containersStore.setState(c))}
            >
              <RefreshCw size={14} className="mr-1" />
              Atualizar
            </Button>
          </div>

          {status.error && (
            <div className="flex items-start gap-3 rounded-md border border-destructive/50 bg-destructive/10 px-4 py-3 text-sm text-destructive">
              <ServerCrash className="h-4 w-4 mt-0.5" />
              <div>
                <p className="font-semibold">Docker indisponível</p>
                <p className="text-xs text-destructive/80">{status.error}</p>
              </div>
            </div>
          )}

          {status.loading ? (
            <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-4">
              {[...Array(3)].map((_, i) => (
                <Skeleton key={i} className="h-44 w-full" />
              ))}
            </div>
          ) : (
            <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-4">
              {containers.map((c) => (
                <ContainerCard key={c.id} container={c} stats={statsMap[c.name]} />
              ))}
            </div>
          )}
        </div>
      </main>
    </div>
  )
}
