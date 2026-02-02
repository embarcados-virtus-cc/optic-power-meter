import { createFileRoute } from '@tanstack/react-router'
import { useStore } from '@tanstack/react-store'
import { RxPower } from '@/components/dashboard/page/RxPower'
import { Info } from '@/components/dashboard/page/Info'
import { Parameters } from '@/components/dashboard/page/Parameters'
import { History } from '@/components/dashboard/page/History'
import { loadingStore } from '@/stores/loadingStore'

export const Route = createFileRoute('/')({
  component: App,
})

function App() {
  const isLoading = useStore(loadingStore)

  return (
    <div className="min-h-max bg-zinc-950">
      <main className="container mx-auto p-6">
        <div className="space-y-6">
          {/* Primeira Linha: RxPower + Parameters */}
          <div className="grid grid-cols-1 lg:grid-cols-2 gap-6 items-stretch">
            <RxPower isLoading={isLoading} />
            <Parameters isLoading={isLoading} />
          </div>

          {/* Segunda Linha: History + Info */}
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
