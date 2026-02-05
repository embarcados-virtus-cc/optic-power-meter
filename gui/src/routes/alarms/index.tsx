import { createFileRoute } from '@tanstack/react-router'
import { useStore } from '@tanstack/react-store'
import { Flags } from '@/components/alarms/page/Flags'
import { Limits } from '@/components/alarms/page/Limits'
import { Separator } from '@/components/ui/separator'
import { loadingStore } from '@/stores/loadingStore'

export const Route = createFileRoute('/alarms/')({
  component: AlarmsPage,
})

function AlarmsPage() {
  const isLoading = useStore(loadingStore)

  return (
    <div className="w-full min-h-max bg-background flex items-center justify-center p-8">
      <div className="flex flex-col xl:flex-row gap-3 items-stretch justify-center w-full max-w-350">
        {/* Flags Section */}
        <div className="w-full xl:w-[40%] min-h-140">
          <Flags isLoading={isLoading} />
        </div>

        {/* Separator */}
        <Separator orientation="vertical" className="hidden xl:block h-auto bg-zinc-700" />
        <Separator orientation="horizontal" className="xl:hidden bg-zinc-700" />

        {/* Limits Section */}
        <div className="w-full xl:w-[60%] min-h-140">
          <Limits isLoading={isLoading} />
        </div>
      </div>
    </div>
  )
}
