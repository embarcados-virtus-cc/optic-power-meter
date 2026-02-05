import { render, screen, fireEvent } from '@testing-library/react'
import { describe, it, expect, vi } from 'vitest'
import { About } from '../page/Sidebar'

describe('About', () => {
  it('renders nothing when closed', () => {
    render(<About open={false} onOpenChange={() => { }} />)
    expect(screen.queryByText(/Sobre o Projeto/i)).not.toBeInTheDocument()
  })
})
