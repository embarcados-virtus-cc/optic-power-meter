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
  <div className="flex flex-col items-center justify-center gap-3 p-4 bg-secondary/50 rounded-lg border border-border">
    {Icon && <Icon className="text-foreground" size={24} />}
    <div className="text-center">
      <p className="text-xs text-foreground uppercase tracking-wide mb-1">
        {label}
      </p>
      <div className="flex items-baseline justify-center gap-1">
        <span className="text-2xl font-bold text-foreground">{value}</span>
        {unit && <span className="text-sm text-foreground">{unit}</span>}
      </div>
    </div>
  </div>
)
