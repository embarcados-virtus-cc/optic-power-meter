import { Outlet, createRootRoute } from '@tanstack/react-router'
import { Header } from '@/components/layout/page/Header'
import { Footer } from '@/components/layout/page/Footer'
import { GlobalAlarmListener } from '@/components/alarms/page/GlobalAlarmListener'

export const Route = createRootRoute({
  component: () => (
    <div className="min-h-screen flex flex-col bg-background">
      <Header />
      <main className="flex-1 flex flex-col justify-center">
        <Outlet />
      </main>
      <Footer />
      <GlobalAlarmListener />
    </div>
  ),
})
