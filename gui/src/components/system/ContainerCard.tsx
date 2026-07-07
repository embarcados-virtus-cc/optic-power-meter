import { useState } from 'react'
import { Play, Square, RotateCcw, FileText, Cpu, HardDrive } from 'lucide-react'
import {
  CardComponent as Card,
  CardContentComponent as CardContent,
  CardHeaderComponent as CardHeader,
  CardTitleComponent as CardTitle,
} from '@/components/ui/card'
import { Badge } from '@/components/ui/badge'
import { Button } from '@/components/ui/button'
import {
  Dialog,
  DialogContent,
  DialogHeader,
  DialogTitle,
  DialogTrigger,
} from '@/components/ui/dialog'
import { api, type ContainerInfo, type ContainerStats } from '@/lib/api'
import { containersStore } from '@/stores/containerStore'

function statusColor(status: string) {
  if (status === 'running') return 'bg-green-500/15 text-green-600 border-green-500/30'
  if (status === 'exited') return 'bg-red-500/15 text-red-600 border-red-500/30'
  return 'bg-yellow-500/15 text-yellow-600 border-yellow-500/30'
}

type Props = {
  container: ContainerInfo
  stats?: ContainerStats
}

export function ContainerCard({ container, stats }: Props) {
  const [logs, setLogs] = useState<string>('')
  const [loadingLogs, setLoadingLogs] = useState(false)
  const [actionLoading, setActionLoading] = useState(false)

  const running = container.status === 'running'

  async function handleAction(action: 'start' | 'stop' | 'restart') {
    setActionLoading(true)
    try {
      if (action === 'start') await api.containerStart(container.name)
      else if (action === 'stop') await api.containerStop(container.name)
      else await api.containerRestart(container.name)
      const updated = await api.containers()
      containersStore.setState(updated)
    } finally {
      setActionLoading(false)
    }
  }

  async function loadLogs() {
    setLoadingLogs(true)
    try {
      const res = await api.containerLogs(container.name, 100)
      setLogs(res.logs)
    } finally {
      setLoadingLogs(false)
    }
  }

  return (
    <Card className="flex flex-col">
      <CardHeader className="pb-2">
        <div className="flex items-start justify-between gap-2">
          <CardTitle className="text-base font-mono">{container.name}</CardTitle>
          <Badge variant="outline" className={statusColor(container.status)}>
            {container.status}
          </Badge>
        </div>
        <p className="text-xs text-muted-foreground font-mono truncate">{container.image}</p>
      </CardHeader>

      <CardContent className="flex flex-col gap-3 flex-1">
        {stats && running && (
          <div className="grid grid-cols-2 gap-2 text-sm">
            <div className="flex items-center gap-1.5 text-muted-foreground">
              <Cpu size={14} />
              <span>{stats.cpu_percent}% CPU</span>
            </div>
            <div className="flex items-center gap-1.5 text-muted-foreground">
              <HardDrive size={14} />
              <span>{stats.memory_mb} MB</span>
            </div>
          </div>
        )}

        <div className="flex flex-wrap gap-2 mt-auto">
          {running ? (
            <>
              <Button
                size="sm"
                variant="outline"
                disabled={actionLoading}
                onClick={() => handleAction('restart')}
              >
                <RotateCcw size={14} className="mr-1" />
                Restart
              </Button>
              <Button
                size="sm"
                variant="destructive"
                disabled={actionLoading}
                onClick={() => handleAction('stop')}
              >
                <Square size={14} className="mr-1" />
                Stop
              </Button>
            </>
          ) : (
            <Button
              size="sm"
              variant="default"
              disabled={actionLoading}
              onClick={() => handleAction('start')}
            >
              <Play size={14} className="mr-1" />
              Start
            </Button>
          )}

          <Dialog>
            <DialogTrigger asChild>
              <Button size="sm" variant="ghost" onClick={loadLogs}>
                <FileText size={14} className="mr-1" />
                Logs
              </Button>
            </DialogTrigger>
            <DialogContent className="max-w-3xl max-h-[80vh] flex flex-col">
              <DialogHeader>
                <DialogTitle className="font-mono">{container.name} — logs</DialogTitle>
              </DialogHeader>
              <pre className="overflow-auto flex-1 text-xs font-mono bg-muted rounded-md p-3 whitespace-pre-wrap break-all">
                {loadingLogs ? 'Carregando...' : logs || '(sem logs)'}
              </pre>
            </DialogContent>
          </Dialog>
        </div>
      </CardContent>
    </Card>
  )
}
