import { render, screen } from '@testing-library/react'
import { describe, it, expect } from 'vitest'
import { CalibrationInfoColumn } from '../page/CalibrationInfoColumn'

describe('CalibrationInfoColumn', () => {
  it('renders info cards', () => {
    render(<CalibrationInfoColumn />)
    expect(screen.getByText('Calibração Interna')).toBeInTheDocument()
    expect(screen.getByText('Calibração Externa')).toBeInTheDocument()
  })
})
