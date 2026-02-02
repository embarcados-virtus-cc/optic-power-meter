import { useState } from 'react'
import { createFileRoute } from '@tanstack/react-router'
import { useStore } from '@tanstack/react-store'
import { calibrationStore } from '@/stores/calibrationStore'
import { CalibrationMode } from '@/components/tuning/page/CalibrationMode'
import { CalibrationInfoColumn } from '@/components/tuning/page/CalibrationInfoColumn'
import { ExternalCalibrationForm } from '@/components/tuning/page/ExternalCalibrationForm'
import { loadingStore } from '@/stores/loadingStore'

export const Route = createFileRoute('/tuning/')({
  component: RouteComponent,
})

function RouteComponent() {
  const { rxPower, txPower, temperature, info, status } =
    useStore(calibrationStore)
  const [activeMode, setActiveMode] = useState<'internal' | 'external'>(
    'internal',
  )
  const isLoading = useStore(loadingStore)

  return (
    <div className="min-h-max bg-zinc-950">
      <main className="container mx-auto p-6">
        <div className="grid grid-cols-1 lg:grid-cols-12 gap-4">
          {/* Col 1: Selector (3 cols) */}
          <div className="lg:col-span-3 h-full">
            <CalibrationMode
              activeMode={activeMode}
              onModeChange={setActiveMode}
              status={status}
              isLoading={isLoading}
            />
          </div>

          {/* Col 2: Info (4 cols) */}
          <div className="lg:col-span-4 h-full">
            <CalibrationInfoColumn isLoading={isLoading} />
          </div>

          {/* Col 3: Form (5 cols) */}
          <div className="lg:col-span-5 h-full">
            <ExternalCalibrationForm
              rxPower={rxPower}
              txPower={txPower}
              temperature={temperature}
              info={info}
              isLoading={isLoading}
            />
          </div>
        </div>
      </main>
    </div>
  )
}
