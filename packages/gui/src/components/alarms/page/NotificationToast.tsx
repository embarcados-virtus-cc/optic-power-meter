import { TriangleAlert, X } from 'lucide-react'
import type { AlarmFlag } from '@/stores/alarmStore'
import { cn } from '@/lib/utils'
import { useEffect, useState } from 'react'

interface NotificationToastProps {
  flag: AlarmFlag
  onDismiss: (id: string) => void
}

export function NotificationToast({ flag, onDismiss }: NotificationToastProps) {
  const [isVisible, setIsVisible] = useState(false)

  useEffect(() => {
    // Trigger entrance animation
    requestAnimationFrame(() => setIsVisible(true))

    // Auto dismiss after 5 seconds
    const timer = setTimeout(() => {
      handleDismiss()
    }, 5000)

    return () => clearTimeout(timer)
  }, [])

  const handleDismiss = () => {
    setIsVisible(false)
    // Wait for exit animation
    setTimeout(() => onDismiss(flag.id), 300)
  }

  const getTypeStyles = (type: 'ALTO' | 'BAIXO') => {
    return type === 'ALTO'
      ? 'border-red-900 text-red-100' // Borda vermelha para ALTO
      : 'border-yellow-600 text-yellow-100' // Borda amarela para BAIXO
  }

  return (
    <div
      className={cn(
        'relative flex items-center gap-3 p-4 rounded-lg border bg-zinc-900 shadow-lg min-w-[300px] max-w-sm transition-all duration-300 ease-in-out',
        getTypeStyles(flag.type),
        isVisible
          ? 'translate-x-0 opacity-100'
          : 'translate-x-[120%] opacity-0',
      )}
      role="alert"
    >
      <div className="flex-shrink-0">
        <TriangleAlert
          size={20}
          className={flag.type === 'ALTO' ? 'text-red-500' : 'text-yellow-500'}
        />
      </div>

      <div className="flex-1 min-w-0 pr-6">
        <div className="flex items-center justify-between mb-1">
          <h4 className="text-sm font-semibold text-slate-200">
            {flag.type === 'ALTO' ? 'Alarme Alto' : 'Alarme Baixo'}
          </h4>
          <span className="text-[10px] text-slate-500 font-mono opacity-70">
            {new Date().toLocaleTimeString()}
          </span>
        </div>
        <p className="text-xs text-slate-400">
          <span className="font-medium text-slate-300">{flag.parameter}</span>{' '}
          está com valor{' '}
          <span className="font-bold text-slate-200">
            {typeof flag.value === 'number'
              ? flag.value.toFixed(2)
              : flag.value}
            {flag.unit}
          </span>
        </p>
      </div>

      <button
        onClick={handleDismiss}
        className="absolute top-2 right-2 p-1 text-slate-500 hover:text-slate-300 transition-colors rounded-sm hover:bg-zinc-800"
        aria-label="Fechar notificação"
      >
        <X size={14} />
      </button>
    </div>
  )
}
