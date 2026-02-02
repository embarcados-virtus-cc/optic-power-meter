import { render, screen } from '@testing-library/react'
import { describe, it, expect, vi } from 'vitest'
import { Parameters } from '../page/Parameters'

// Mock store
vi.mock('@tanstack/react-store', () => ({
  useStore: () => ({
    corrente: 25.5,
    tensao: 3.3,
    temperatura: 45.2,
    qualidadeSinal: 'OK',
  }),
}))

describe('Parameters', () => {
  it('renders parameter values', () => {
    render(<Parameters isLoading={false} />)
    expect(screen.getByText('25.5')).toBeInTheDocument()
    expect(screen.getByText('3.3')).toBeInTheDocument()
    expect(screen.getByText('45.2')).toBeInTheDocument()
  })
})
