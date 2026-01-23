-- Schema para Supabase/Postgres: medições do módulo SFP (power meter)
-- Execute no SQL Editor do Supabase.

create table if not exists public.sfp_measurements (
  id bigserial primary key,
  created_at timestamptz not null default now(),

  -- Identificação do módulo (EEPROM A0h base)
  identifier integer null,
  ext_identifier integer null,
  connector integer null,
  encoding integer null,
  vendor_name text null,
  vendor_pn text null,
  vendor_rev text null,
  cc_base_valid boolean null,

  -- Métricas (diagnósticos A2h quando estiverem disponíveis)
  rx_power_dbm double precision null,
  temperature_c double precision null,
  voltage_v double precision null,
  bias_ma double precision null,
  signal_quality text null
);

create index if not exists sfp_measurements_created_at_idx
  on public.sfp_measurements (created_at desc);


