import { render, screen } from '@testing-library/react'
import { describe, it, expect, vi } from 'vitest'
import { History } from '../page/History'

// Mock store
vi.mock('@tanstack/react-store', () => ({
  useStore: () => [-10, -5, -8, -12],
}))

describe('History', () => {
  it('renders with title', () => {
    render(<History isLoading={false} />)
    expect(
      screen.getByText(/Histórico de Potência Óptica/i),
    ).toBeInTheDocument()
  })
})
