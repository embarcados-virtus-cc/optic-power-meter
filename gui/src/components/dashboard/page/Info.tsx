import { Package, Plus } from 'lucide-react'
import { Skeleton } from '../../ui/skeleton'
import { Label } from '../../ui/label'
import { Badge } from '../../ui/badge'
import { Button } from '../../ui/button'
import { useState } from 'react'
import { StaticInfoModal } from '../modal/StaticInfoModal'
import {
  CardComponent,
  CardContentComponent,
  CardHeaderComponent,
} from '@/components/ui/card'

export function Info({ isLoading }: { isLoading: boolean }) {
  const [modalOpen, setModalOpen] = useState(false)

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
              <Skeleton className="w-6 h-6 bg-muted rounded-md" />
              <Skeleton className="w-64 h-6 bg-muted rounded-md" />
            </>
          ) : (
            <>
              <div className="flex items-center gap-3 flex-1">
                <Package className="text-foreground" size={24} />
                <Label className="text-lg font-bold uppercase tracking-wider text-foreground">
                  Módulo SFP e Informação de DDM
                </Label>
              </div>
              <Button variant="ghost" size="icon" className="h-8 w-8 text-muted-foreground hover:text-foreground" onClick={() => setModalOpen(true)}
              >
                <Plus size={18} />
              </Button>
            </>
          )}
        </div>
      </CardHeaderComponent>

      <StaticInfoModal open={modalOpen} onOpenChange={setModalOpen} />

      <CardContentComponent className="min-h-59 2xl:min-h-66 flex-1 flex flex-col">
        {isLoading ? (
          <div className="space-y-4 flex-1">
            {/* Primeira Seção */}
            <div className="grid grid-cols-2 gap-4">
              {[1, 2, 3, 4].map((i) => (
                <div key={i} className="flex flex-col gap-2">
                  <Skeleton className="w-24 h-6 rounded-md bg-muted" />
                  <Skeleton className="w-32 h-5 bg-muted" />
                </div>
              ))}
            </div>
            <div className="w-full h-px bg-background my-2" />
            {/* Segunda Seção */}
            <div className="grid grid-cols-2 gap-4 pt-4">
              {[1, 2].map((i) => (
                <div key={i} className="flex flex-col gap-2">
                  <Skeleton className="w-32 h-6 rounded-md bg-muted" />
                  <Skeleton className="w-24 h-5 bg-muted" />
                </div>
              ))}
            </div>
          </div>
        ) : (
          <div className="space-y-4 flex-1">
            {/* Primeira Seção */}
            <div className="grid grid-cols-1 sm:grid-cols-2 gap-4">
              {firstSection.map((info, index) => (
                <div key={index} className="flex flex-col gap-2">
                  <Badge variant="outline" className="w-fit rounded-md bg-secondary border-none text-foreground">
                    {info.label}
                  </Badge>
                  <span className="text-sm font-medium text-foreground">
                    {info.value}
                  </span>
                </div>
              ))}
            </div>

            {/* Linha Divisória */}
            <div className="w-full h-px bg-background/50 my-2" />

            {/* Segunda Seção */}
            <div className="grid grid-cols-1 sm:grid-cols-2 gap-4 pt-4">
              {secondSection.map((info, index) => (
                <div key={index} className="flex flex-col gap-2">
                  <Badge variant="outline" className="w-fit rounded-md bg-secondary border-none text-foreground">
                    {info.label}
                  </Badge>
                  <span className="text-sm font-medium text-foreground">
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
