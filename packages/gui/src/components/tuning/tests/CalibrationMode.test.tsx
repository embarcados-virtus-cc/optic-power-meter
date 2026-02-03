import { render, screen, fireEvent } from '@testing-library/react'
import { describe, it, expect, vi } from 'vitest'
import { CalibrationMode } from '../page/CalibrationMode'

describe('CalibrationMode', () => {
  it('renders with active internal mode', () => {
    render(
      <CalibrationMode
        activeMode="internal"
        onModeChange={() => { }}
        isLoading={false}
      />,
    )
    expect(screen.getByText('CALIBRAÇÃO INTERNA')).toBeInTheDocument()
    // Check for active style or element if necessary
  })

  it('calls onModeChange when clicking external', () => {
    const onModeChange = vi.fn()
    render(
      <CalibrationMode
        activeMode="internal"
        onModeChange={onModeChange}
        isLoading={false}
      />,
    )

    fireEvent.click(screen.getByText('CALIBRAÇÃO EXTERNA'))
    expect(onModeChange).toHaveBeenCalledWith('external')
  })

  it('displays status badges when status provided', () => {
    const status = {
      rx: true,
      tx: false,
      bias: true,
      voltage: true,
      temp: true,
    }
    render(
      <CalibrationMode
        activeMode="internal"
        onModeChange={() => { }}
        status={status}
        isLoading={false}
      />,
    )

    expect(screen.getByText('Status da Calibração')).toBeInTheDocument()
    expect(screen.getByText('RX')).toBeInTheDocument()
  })
})
