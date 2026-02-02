import React from 'react'

export const DataPlaceholder = ({
  label,
  value,
  unit,
  icon: Icon,
}: {
  label: string
  value: string
  unit: string
  icon?: React.ComponentType<{ className?: string; size?: number }>
}) => (
  <div className="flex flex-col items-center justify-center gap-3 p-4 bg-zinc-800/50 rounded-lg border border-slate-700">
    {Icon && <Icon className="text-slate-300" size={24} />}
    <div className="text-center">
      <p className="text-xs text-slate-300 uppercase tracking-wide mb-1">
        {label}
      </p>
      <div className="flex items-baseline justify-center gap-1">
        <span className="text-2xl font-bold text-slate-300">{value}</span>
        {unit && <span className="text-sm text-slate-300">{unit}</span>}
      </div>
    </div>
  </div>
)
