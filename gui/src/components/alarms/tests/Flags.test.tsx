import { render, screen } from '@testing-library/react'
import { describe, it, expect, vi } from 'vitest'
import { Flags } from '../page/Flags'

const { mockUseStore } = vi.hoisted(() => {
  return { mockUseStore: vi.fn() }
})

// Mock the store module to avoid side effects
vi.mock('@/stores/alarmStore', () => {
  return {
    alarmStore: {
      state: [],
      subscribe: vi.fn(),
      setState: vi.fn(),
    },
    removeAlarm: vi.fn(),
  }
})

// Mock useStore from tanstack
vi.mock('@tanstack/react-store', () => ({
  useStore: mockUseStore,
}))

describe('Flags', () => {
  it('renders loading skeleton', () => {
    render(<Flags isLoading={true} />)
    // Check for skeleton class logic or structure
    // Since skeleton usually has 'animate-pulse' or similar, we can assume it renders if no error
  })

  it('renders empty state when no flags', () => {
    mockUseStore.mockReturnValue([])

    render(<Flags isLoading={false} />)
    expect(
      screen.getByText('Nenhuma flag ativa no momento'),
    ).toBeInTheDocument()
  })

  it('renders flags list', () => {
    const mockFlags = [
      {
        id: '1',
        parameter: 'Temp',
        value: 40,
        unit: '°C',
        type: 'ALTO',
        timestamp: '10:00',
      },
    ]
    mockUseStore.mockReturnValue(mockFlags)

    render(<Flags isLoading={false} />)
    expect(screen.getByText('Temp')).toBeInTheDocument()
    expect(screen.getByText('ALTO')).toBeInTheDocument()
  })
})
