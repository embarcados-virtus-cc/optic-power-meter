import { BarChart3, FileText, Plug, Thermometer, Zap } from 'lucide-react'
import { useStore } from '@tanstack/react-store'
import { Skeleton } from '../../ui/skeleton'
import { Label } from '../../ui/label'
import {
  CardComponent,
  CardContentComponent,
  CardHeaderComponent,
} from '@/components/ui/card'
import { parametersStore } from '@/stores/dataStores'

export function Parameters({ isLoading }: { isLoading: boolean }) {
  const paramsData = useStore(parametersStore)

  const params = [
    {
      label: 'Corrente',
      value: paramsData.corrente.toString(),
      unit: 'mA',
      icon: Plug,
    },
    {
      label: 'Tensão',
      value: paramsData.tensao.toString(),
      unit: 'V',
      icon: Zap,
    },
    {
      label: 'Temperatura',
      value: paramsData.temperatura,
      unit: '°C',
      icon: Thermometer,
    },
    {
      label: 'Qualidade do Sinal',
      value: paramsData.qualidadeSinal,
      unit: '',
      icon: BarChart3,
    },
  ]

  return (
    <CardComponent className="h-full w-full flex flex-col">
      <CardHeaderComponent>
        <div className="flex items-center gap-3">
          {isLoading ? (
            <>
              <Skeleton className="w-6 h-6 bg-muted rounded-md" />
              <Skeleton className="w-48 h-6 bg-muted rounded-md" />
            </>
          ) : (
            <>
              <FileText className="text-foreground" size={24} />
              <Label className="text-lg font-bold uppercase tracking-wider text-foreground">
                Parâmetros do Módulo
              </Label>
            </>
          )}
        </div>
      </CardHeaderComponent>
      <CardContentComponent className="min-h-48 flex-1">
        {isLoading ? (
          <div className="grid grid-cols-2 gap-4 h-full items-stretch">
            {[1, 2, 3, 4].map((i) => (
              <div
                key={i}
                className="flex items-center justify-between p-6 bg-secondary rounded-lg h-full min-h-24"
              >
                <div className="flex flex-col gap-2">
                  <Skeleton className="w-6 h-6 bg-muted" />
                  <Skeleton className="w-20 h-4 bg-muted" />
                </div>
                <div className="flex items-baseline gap-1">
                  <Skeleton className="w-12 h-8 bg-muted" />
                  <Skeleton className="w-8 h-5 bg-muted" />
                </div>
              </div>
            ))}
          </div>
        ) : (
          <div className="grid grid-cols-2 gap-4 h-full items-stretch">
            {params.map((param, index) => (
              <div
                key={index}
                className="flex items-center justify-between p-6 bg-secondary rounded-lg min-h-24"
              >
                <div className="flex flex-col gap-2">
                  <param.icon className="text-foreground" size={24} />
                  <Label className="text-xs uppercase tracking-wide text-foreground">
                    {param.label}
                  </Label>
                </div>
                <div className="flex items-baseline gap-1">
                  <span className="text-2xl font-bold text-foreground">
                    {param.value}
                  </span>
                  {param.unit && (
                    <span className="text-sm text-foreground">{param.unit}</span>
                  )}
                </div>
              </div>
            ))}
          </div>
        )}
      </CardContentComponent>
    </CardComponent>
  )
}
