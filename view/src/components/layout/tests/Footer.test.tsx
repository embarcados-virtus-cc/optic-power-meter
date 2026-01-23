import { render, screen, waitFor } from '@testing-library/react'
import { describe, it, expect } from 'vitest'
import { Footer } from '../page/Footer'

describe('Footer', () => {
  it('renders loading skeleton initially', () => {
    render(<Footer />)
  })

  it('renders project info after loading', async () => {
    render(<Footer />)
    await waitFor(() => {
      expect(
        screen.getByText(/Medidor de Potência Óptica \(2026\)/i),
      ).toBeInTheDocument()
    })
  })

  it('renders github link', async () => {
    render(<Footer />)
    await waitFor(() => {
      const link = screen.getByLabelText('GitHub')
      expect(link).toHaveAttribute(
        'href',
        'https://github.com/embarcados-virtus-cc/optic-power-meter-gui',
      )
    })
  })
})
