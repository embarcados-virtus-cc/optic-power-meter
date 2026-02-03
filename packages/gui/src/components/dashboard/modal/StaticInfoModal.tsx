import { useEffect, useState } from 'react'
import {
    Dialog,
    DialogContent,
    DialogDescription,
    DialogHeader,
    DialogTitle,
} from '@/components/ui/dialog'
import { api, type SfpStaticData } from '@/lib/api'
import { Separator } from '@/components/ui/separator'
import { Badge } from '@/components/ui/badge'
import { Loader2 } from 'lucide-react'

interface StaticInfoModalProps {
    open: boolean
    onOpenChange: (open: boolean) => void
}

export function StaticInfoModal({ open, onOpenChange }: StaticInfoModalProps) {
    const [data, setData] = useState<SfpStaticData | null>(null)
    const [loading, setLoading] = useState(false)
    const [error, setError] = useState<string | null>(null)

    useEffect(() => {
        if (open) {
            loadData()
        }
    }, [open])

    async function loadData() {
        try {
            setLoading(true)
            setError(null)
            const res = await api.static()
            setData(res)
        } catch (err) {
            console.error(err)
            setError('Falha ao carregar dados estáticos.')
        } finally {
            setLoading(false)
        }
    }

    return (
        <Dialog open={open} onOpenChange={onOpenChange}>
            <DialogContent className="max-w-4xl max-h-[80vh] flex flex-col">
                <DialogHeader>
                    <DialogTitle>Informações Estáticas do Módulo SFP</DialogTitle>
                    <DialogDescription>
                        Detalhes completos da memória A0h
                    </DialogDescription>
                </DialogHeader>

                <div className="flex-1 overflow-hidden min-h-0 relative">
                    {loading ? (
                        <div className="flex h-full items-center justify-center">
                            <Loader2 className="animate-spin text-primary" size={32} />
                        </div>
                    ) : error ? (
                        <div className="flex h-full items-center justify-center text-destructive">
                            {error}
                        </div>
                    ) : !data ? (
                        <div className="flex h-full items-center justify-center text-muted-foreground">
                            Nenhum dado disponível
                        </div>
                    ) : (
                        <div className="h-full overflow-y-auto pr-2 space-y-6">

                            {/* Identificação */}
                            <Section title="Identificação">
                                <Grid>
                                    <Item label="Identifier" value={data.identifier} sub={data.identifier_type} />
                                    <Item label="Extended Identifier" value={data.ext_identifier} valid={data.ext_identifier_valid} />
                                    <Item label="Connector" value={data.connector} sub={data.connector_type} />
                                    <Item label="Encoding" value={data.encoding} />
                                </Grid>
                            </Section>

                            <Separator />

                            {/* Fabricante */}
                            <Section title="Fabricante">
                                <Grid>
                                    <Item label="Vendor Name" value={data.vendor_name} valid={data.vendor_name_valid} />
                                    <Item label="Vendor P/N" value={data.vendor_pn} valid={data.vendor_pn_valid} />
                                    <Item label="Vendor Rev" value={data.vendor_rev} />
                                    <Item label="Vendor OUI" value={data.vendor_oui?.map(x => x.toString(16).padStart(2, '0')).join(':').toUpperCase()} valid={data.vendor_oui_valid} />
                                </Grid>
                            </Section>

                            <Separator />

                            {/* Taxa e Comprimento */}
                            <Section title="Taxa e Cabo">
                                <Grid>
                                    <Item label="Nominal Rate" value={`${data.nominal_rate_mbd} MBd`} sub={`Status: ${data.nominal_rate_status}`} />
                                    <Item label="Link Length (SMF)" value={`${data.smf_length_km} km`} />
                                    <Item label="Link Length (OM2)" value={`${data.om2_length_m} m`} />
                                    <Item label="Link Length (OM1)" value={`${data.om1_length_m} m`} />
                                    <Item label="Link Length (OM4/Cu)" value={`${data.om4_or_copper_length_m} m`} />
                                    <Item label="Wavelength" value={data.wavelength_nm ? `${data.wavelength_nm} nm` : '-'} />
                                </Grid>
                            </Section>

                            <Separator />

                            {/* Compliance Codes */}
                            {data.compliance_codes && (
                                <Section title="Compliance Codes">
                                    <div className="flex flex-wrap gap-2">
                                        {Object.entries(data.compliance_codes).map(([_, codes]) => {
                                            if (!codes) return null
                                            return Object.entries(codes).map(([code, active]) => (
                                                active ? (
                                                    <Badge key={code} variant="secondary" className="text-xs">
                                                        {code.replace(/_/g, ' ')}
                                                    </Badge>
                                                ) : null
                                            ))
                                        })}
                                    </div>
                                </Section>
                            )}

                            <Separator />

                            {/* Outros */}
                            <Section title="Outros">
                                <Grid>
                                    <Item label="CC_BASE Checksum" value={data.cc_base} valid={data.cc_base_valid} />
                                </Grid>
                            </Section>

                        </div>
                    )}
                </div>
            </DialogContent>
        </Dialog>
    )
}

function Section({ title, children }: { title: string; children: React.ReactNode }) {
    return (
        <div className="space-y-3">
            <h4 className="text-sm font-semibold text-foreground uppercase tracking-wider">{title}</h4>
            {children}
        </div>
    )
}

function Grid({ children }: { children: React.ReactNode }) {
    return <div className="grid grid-cols-1 sm:grid-cols-2 md:grid-cols-4 gap-4">{children}</div>
}

function Item({ label, value, sub, valid }: { label: string; value: string | number | undefined | null; sub?: string | number; valid?: boolean }) {
    return (
        <div className="flex flex-col">
            <span className="text-xs text-muted-foreground">{label}</span>
            <div className="flex items-center gap-2">
                <span className="font-medium text-sm truncate" title={String(value)}>
                    {value !== undefined && value !== null ? value : 'N/A'}
                </span>
                {valid === false && (
                    <Badge variant="destructive" className="h-4 px-1 text-[10px]">INV</Badge>
                )}
                {valid === true && (
                    <Badge variant="outline" className="h-4 px-1 text-[10px] text-green-500 border-green-500">OK</Badge>
                )}
            </div>
            {sub && <span className="text-xs text-muted-foreground/70">{sub}</span>}
        </div>
    )
}
