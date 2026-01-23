import { Package } from 'lucide-react'
import { Skeleton } from '../../ui/skeleton'
import { Label } from '../../ui/label'
import { Badge } from '../../ui/badge'
import {
  CardComponent,
  CardContentComponent,
  CardHeaderComponent,
} from '@/components/ui/card'

export function Info({ isLoading }: { isLoading: boolean }) {
  const firstSection = [
    { label: 'Vendor', value: 'FINISAR CORP' },
    { label: 'Serial Number', value: 'UVW2024ABC' },
    { label: 'Connector', value: 'LC' },
    { label: 'Part Number', value: 'FTLX8574D3BCL' },
  ]

  const secondSection = [
    { label: 'Comprimento de Onda', value: '850 nm' },
    { label: 'Tipo', value: '10GBASE-SR' },
  ]

  return (
    <CardComponent className="w-full flex flex-col">
      <CardHeaderComponent>
        <div className="flex items-center gap-3">
          {isLoading ? (
            <>
              <Skeleton className="w-6 h-6 bg-zinc-800 rounded-md" />
              <Skeleton className="w-64 h-6 bg-zinc-800 rounded-md" />
            </>
          ) : (
            <>
              <Package className="text-slate-300" size={24} />
              <Label className="text-lg font-bold uppercase tracking-wider text-slate-300">
                Módulo SFP e Informação de DDM
              </Label>
            </>
          )}
        </div>
      </CardHeaderComponent>
      <CardContentComponent className="min-h-59 2xl:min-h-66 flex-1 flex flex-col">
        {isLoading ? (
          <div className="space-y-4 flex-1">
            {/* Primeira Seção */}
            <div className="grid grid-cols-2 gap-4">
              {[1, 2, 3, 4].map((i) => (
                <div key={i} className="flex flex-col gap-2">
                  <Skeleton className="w-24 h-6 rounded-md bg-zinc-800" />
                  <Skeleton className="w-32 h-5 bg-zinc-800" />
                </div>
              ))}
            </div>
            <div className="w-full h-px bg-zinc-950 my-2" />
            {/* Segunda Seção */}
            <div className="grid grid-cols-2 gap-4 pt-4">
              {[1, 2].map((i) => (
                <div key={i} className="flex flex-col gap-2">
                  <Skeleton className="w-32 h-6 rounded-md bg-zinc-800" />
                  <Skeleton className="w-24 h-5 bg-zinc-800" />
                </div>
              ))}
            </div>
          </div>
        ) : (
          <div className="space-y-4 flex-1">
            {/* Primeira Seção */}
            <div className="grid grid-cols-2 gap-4">
              {firstSection.map((info, index) => (
                <div key={index} className="flex flex-col gap-2">
                  <Badge
                    variant="outline"
                    className="w-fit rounded-md bg-zinc-800 border-none text-slate-300"
                  >
                    {info.label}
                  </Badge>
                  <span className="text-sm font-medium text-slate-300">
                    {info.value}
                  </span>
                </div>
              ))}
            </div>

            {/* Linha Divisória */}
            <div className="w-full h-px bg-zinc-950/50 my-2" />

            {/* Segunda Seção */}
            <div className="grid grid-cols-2 gap-4 pt-4">
              {secondSection.map((info, index) => (
                <div key={index} className="flex flex-col gap-2">
                  <Badge
                    variant="outline"
                    className="w-fit rounded-md bg-zinc-800 border-none text-slate-300"
                  >
                    {info.label}
                  </Badge>
                  <span className="text-sm font-medium text-slate-300">
                    {info.value}
                  </span>
                </div>
              ))}
            </div>
          </div>
        )}
      </CardContentComponent>
    </CardComponent>
  )
}
