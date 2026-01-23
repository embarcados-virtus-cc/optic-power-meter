import { render, screen, fireEvent, waitFor } from '@testing-library/react'
import { describe, it, expect, vi } from 'vitest'
import { NotificationToast } from '../page/NotificationToast'
import type { AlarmFlag } from '@/stores/alarmStore'

describe('NotificationToast', () => {
  const mockFlag: AlarmFlag = {
    id: '1',
    parameter: 'Temp',
    value: 40,
    unit: '°C',
    type: 'ALTO',
    timestamp: '10:00:00',
  }

  it('renders correctly with flag data', () => {
    render(<NotificationToast flag={mockFlag} onDismiss={() => {}} />)
    expect(screen.getByText('Alarme Alto')).toBeInTheDocument()
    expect(screen.getByText('Temp')).toBeInTheDocument()
    expect(screen.getByText(/40.*°C/)).toBeInTheDocument()
  })

  it('calls onDismiss after close button click (with animation delay)', async () => {
    const onDismiss = vi.fn()
    render(<NotificationToast flag={mockFlag} onDismiss={onDismiss} />)

    const closeBtn = screen.getByLabelText('Fechar notificação')
    fireEvent.click(closeBtn)

    await waitFor(
      () => {
        expect(onDismiss).toHaveBeenCalledWith('1')
      },
      { timeout: 1000 },
    )
  })
})
