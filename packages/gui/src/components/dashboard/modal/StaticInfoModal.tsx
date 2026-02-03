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
            // Silently handle error to show template
            setError('Falha ao carregar dados. Mostrando template.')
        } finally {
            setLoading(false)
        }
    }

    // Helper para garantir que temos um objeto para renderizar, mesmo que vazio
    const displayData = data || {}

    return (
        <Dialog open={open} onOpenChange={onOpenChange}>
            <DialogContent className="w-full h-full max-h-[95vh] sm:max-h-[85vh] sm:max-w-4xl flex flex-col p-4 sm:p-6">
                <DialogHeader className="mb-2">
                    <div className="flex flex-col sm:flex-row items-center justify-between sm:pr-8 gap-2 sm:gap-0">
                        <div className="text-center sm:text-left">
                            <DialogTitle className="text-lg sm:text-xl">Informações Estáticas do Módulo SFP</DialogTitle>
                            <DialogDescription className="text-sm">
                                Detalhes completos da memória A0h
                            </DialogDescription>
                        </div>
                        <div className="flex items-center gap-2">
                            {loading && <Loader2 className="animate-spin text-primary" size={20} />}
                            {!loading && error && <Badge variant="destructive">Offline</Badge>}
                        </div>
                    </div>
                </DialogHeader>

                <div className="flex-1 overflow-hidden min-h-0 relative">
                    <div className="h-full overflow-y-auto pr-2 space-y-4 sm:space-y-6">

                        {/* Identificação Básica */}
                        <Section title="Identificação Básica">
                            <Grid>
                                <Item label="Identifier" value={displayData.identifier} sub={displayData.identifier_type} />
                                <Item label="Ext. Identifier" value={displayData.ext_identifier} valid={displayData.ext_identifier_valid} />
                                <Item label="Connector" value={displayData.connector} sub={displayData.connector_type} />
                                <Item label="Encoding" value={displayData.encoding} />
                            </Grid>
                        </Section>

                        <Separator />

                        {/* Taxas e Comprimento */}
                        <Section title="Taxas e Características">
                            <Grid>
                                <Item label="Nominal Rate" value={displayData.nominal_rate_mbd !== undefined ? `${displayData.nominal_rate_mbd} MBd` : undefined} sub={displayData.nominal_rate_status !== undefined ? `Status: ${displayData.nominal_rate_status}` : undefined} />
                                <Item label="Rate Identifier" value={displayData.rate_identifier} />
                                <Item label="SMF Length" value={displayData.smf_length_km !== undefined ? `${displayData.smf_length_km} km` : undefined} sub={displayData.smf_length_status !== undefined ? `Status: ${displayData.smf_length_status}` : undefined} />
                                <Item label="SMF Attenuation" value={displayData.smf_attenuation_db_per_100m !== undefined ? `${displayData.smf_attenuation_db_per_100m} dB/100m` : undefined} />
                                <Item label="OM2 Length" value={displayData.om2_length_m !== undefined ? `${displayData.om2_length_m} m` : undefined} sub={displayData.om2_length_status !== undefined ? `Status: ${displayData.om2_length_status}` : undefined} />
                                <Item label="OM1 Length" value={displayData.om1_length_m !== undefined ? `${displayData.om1_length_m} m` : undefined} sub={displayData.om1_length_status !== undefined ? `Status: ${displayData.om1_length_status}` : undefined} />
                                <Item label="OM4/Copper Length" value={displayData.om4_or_copper_length_m !== undefined ? `${displayData.om4_or_copper_length_m} m` : undefined} sub={displayData.om4_or_copper_length_status !== undefined ? `Status: ${displayData.om4_or_copper_length_status}` : undefined} />
                                <Item label="Wavelength" value={displayData.wavelength_nm !== undefined ? `${displayData.wavelength_nm} nm` : undefined} />
                            </Grid>
                        </Section>

                        <Separator />

                        {/* Compliance */}
                        <Section title="Compliance">
                            <Grid>
                                <div className="col-span-1 sm:col-span-2 md:col-span-4 flex flex-col items-center sm:items-start">
                                    <span className="text-xs text-muted-foreground block mb-2 text-center sm:text-left">Codes</span>
                                    <div className="flex flex-wrap gap-2 justify-center sm:justify-start">
                                        {displayData.compliance_codes ? (
                                            Object.entries(displayData.compliance_codes).map(([_, codes]) => {
                                                if (!codes) return null
                                                return Object.entries(codes).map(([code, active]) => (
                                                    active ? (
                                                        <Badge key={code} variant="secondary" className="text-xs">
                                                            {code.replace(/_/g, ' ')}
                                                        </Badge>
                                                    ) : null
                                                ))
                                            })
                                        ) : (
                                            <span className="text-xs text-muted-foreground italic">Nenhum código detectado</span>
                                        )}
                                    </div>
                                </div>
                                <Item label="Variant" value={displayData.variant === 0 ? 'OPTICAL' : displayData.variant === 1 ? 'PASSIVE_CABLE' : displayData.variant === 2 ? 'ACTIVE_CABLE' : displayData.variant} />
                                <Item label="Cable Compliance" value={displayData.cable_compliance} />
                                <Item label="Ext. Compliance" value={displayData.ext_compliance_code} sub={displayData.ext_compliance_desc} />
                            </Grid>
                        </Section>

                        <Separator />

                        {/* Fabricante */}
                        <Section title="Fabricante">
                            <Grid>
                                <Item label="Vendor Name" value={displayData.vendor_name} valid={displayData.vendor_name_valid} />
                                <Item label="Vendor P/N" value={displayData.vendor_pn} valid={displayData.vendor_pn_valid} />
                                <Item label="Vendor Rev" value={displayData.vendor_rev} />
                                <Item label="Vendor OUI" value={displayData.vendor_oui?.map((x: number) => x.toString(16).padStart(2, '0')).join(':').toUpperCase()} valid={displayData.vendor_oui_valid} />
                                <Item label="OUI (UInt32)" value={displayData.vendor_oui_u32} />
                            </Grid>
                        </Section>

                        <Separator />

                        {/* Outros Checksums */}
                        <Section title="Outros">
                            <Grid>
                                <Item label="CC_BASE Checksum" value={displayData.cc_base} valid={displayData.cc_base_valid} />
                                <Item label="Fibre Channel Speed 2" value={displayData.fc_speed_2} valid={displayData.fc_speed_2_valid} />
                            </Grid>
                        </Section>
                    </div>
                </div>
            </DialogContent>
        </Dialog>
    )
}

function Section({ title, children }: { title: string; children: React.ReactNode }) {
    return (
        <div className="space-y-3">
            <h4 className="text-sm font-semibold text-foreground uppercase tracking-wider text-center sm:text-left">{title}</h4>
            {children}
        </div>
    )
}

function Grid({ children }: { children: React.ReactNode }) {
    return <div className="grid grid-cols-1 sm:grid-cols-2 md:grid-cols-4 gap-4">{children}</div>
}

function Item({ label, value, sub, valid }: { label: string; value: string | number | undefined | null; sub?: string | number; valid?: boolean }) {
    return (
        <div className="flex flex-col items-center sm:items-start text-center sm:text-left">
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
