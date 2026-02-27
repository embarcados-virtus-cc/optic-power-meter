import { createFileRoute } from '@tanstack/react-router'
import { useStore } from '@tanstack/react-store'
import { AlertCircle } from 'lucide-react'
import { RxPower } from '@/components/dashboard/page/RxPower'
import { Info } from '@/components/dashboard/page/Info'
import { Parameters } from '@/components/dashboard/page/Parameters'
import { History } from '@/components/dashboard/page/History'
import { loadingStore } from '@/stores/loadingStore'
import { statusStore } from '@/stores/dataStores'

export const Route = createFileRoute('/')({
  component: App,
})

function App() {
  const isLoading = useStore(loadingStore)
  const status = useStore(statusStore)

  return (
    <div className="min-h-max bg-background">
      <main className="container mx-auto p-6">
        <div className="space-y-6">
          {status && !status.online && (
            <div className="flex items-start gap-3 rounded-md border border-destructive/50 bg-destructive/10 px-4 py-3 text-sm text-destructive">
              <AlertCircle className="h-4 w-4 mt-0.5" />
              <div>
                <p className="font-semibold">Sem dados do módulo SFP</p>
                <p className="text-xs text-destructive/80">
                  Nenhum módulo SFP detectado ou API/daemon indisponível.
                </p>
              </div>
            </div>
          )}
          <div className="grid grid-cols-1 lg:grid-cols-2 gap-6 items-stretch">
            <RxPower isLoading={isLoading} />
            <Parameters isLoading={isLoading} />
          </div>
          <div className="grid grid-cols-1 lg:grid-cols-12 gap-6 items-stretch">
            <div className="lg:col-span-8">
              <History isLoading={isLoading} />
            </div>
            <div className="lg:col-span-4">
              <Info isLoading={isLoading} />
            </div>
          </div>
        </div>
      </main>
    </div>
  )
}
