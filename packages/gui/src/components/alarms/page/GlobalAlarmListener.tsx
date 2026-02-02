import { useStore } from '@tanstack/react-store'
import { useLocation } from '@tanstack/react-router'
import { alarmStore } from '@/stores/alarmStore'
import type { AlarmFlag } from '@/stores/alarmStore'
import { NotificationToast } from './NotificationToast'
import { useEffect, useRef, useState } from 'react'

export function GlobalAlarmListener() {
  const flags = useStore(alarmStore)
  const location = useLocation()
  const [activeNotifications, setActiveNotifications] = useState<AlarmFlag[]>(
    [],
  )
  const prevFlagsLengthRef = useRef(flags.length)

  // Check if we are on the alarms page
  const isAlarmsPage = location.pathname === '/alarms'

  useEffect(() => {
    // Only trigger if we have MORE flags than before (new alarm added)
    if (flags.length > prevFlagsLengthRef.current) {
      const newFlagsCount = flags.length - prevFlagsLengthRef.current
      const newAlarms = flags.slice(-newFlagsCount)

      if (!isAlarmsPage) {
        setActiveNotifications((prev) => [...prev, ...newAlarms])
      }
    }

    // updates the ref
    prevFlagsLengthRef.current = flags.length
  }, [flags, isAlarmsPage])

  const dismissNotification = (id: string) => {
    setActiveNotifications((prev) => prev.filter((n) => n.id !== id))
  }

  // If there are no notifications, don't render anything
  if (activeNotifications.length === 0) return null

  return (
    <div className="fixed bottom-4 right-4 z-50 flex flex-col gap-2 pointer-events-none">
      {/* Enable pointer events for the toasts themselves */}
      {activeNotifications.map((flag) => (
        <div key={flag.id} className="pointer-events-auto">
          <NotificationToast flag={flag} onDismiss={dismissNotification} />
        </div>
      ))}
    </div>
  )
}
