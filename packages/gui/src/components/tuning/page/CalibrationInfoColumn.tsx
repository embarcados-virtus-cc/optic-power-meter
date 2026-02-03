import { ServerCog } from 'lucide-react'
import { Skeleton } from '../../ui/skeleton'

interface CalibrationInfoCardProps {
  title: string
  description: string
  isLoading: boolean
}

interface CalibrationInfoColumnProps {
  isLoading: boolean
}

export function CalibrationInfoColumn({
  isLoading,
}: CalibrationInfoColumnProps) {
  return (
    <div className="flex flex-col gap-6 h-full">
      <InfoCard
        title="Calibração Interna"
        description="Utiliza os valores de calibração internos armazenados na EEPROM do módulo SFP+. Os dados são calculados automaticamente pelo firmware do transceptor."
        isLoading={isLoading}
      />
      <InfoCard
        title="Calibração Externa"
        description="Permite aplicar coeficientes de calibração externos. Útil para compensar desvios ou calibrar contra um padrão de referência."
        isLoading={isLoading}
      />
    </div>
  )
}

function InfoCard({ title, description, isLoading }: CalibrationInfoCardProps) {
  return (
    <div className="bg-secondary/50 border-border flex-1 rounded-xl p-5 border flex flex-col items-center justify-center text-center shadow-lg">
      {isLoading ? (
        <>
          <div className="flex gap-3 justify-center mb-8 w-full">
            <Skeleton className="w-6 h-6 bg-muted rounded-md" />
            <Skeleton className="w-48 h-6 bg-muted rounded-md" />
          </div>
          <Skeleton className="w-full max-w-sm h-32 bg-muted/50 rounded-xl" />
        </>
      ) : (
        <>
          <h3 className="flex gap-3 justify-center text-[1.25rem] font-bold text-foreground mb-8 uppercase">
            <ServerCog className="mt-[0.150rem]" />
            {title}
          </h3>
          <div className="bg-background/50 border border-border/50 p-4 rounded-xl text-[1rem] text-muted-foreground leading-relaxed max-w-sm">
            {description}
          </div>
        </>
      )}
    </div>
  )
}
