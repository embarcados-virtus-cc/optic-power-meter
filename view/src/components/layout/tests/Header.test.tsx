import { render, screen, waitFor } from '@testing-library/react'
import { describe, it, expect } from 'vitest'
import { Header } from '../page/Header'
import {
  createRootRoute,
  createRouter,
  RouterProvider,
} from '@tanstack/react-router'

// Mock resize observer
global.ResizeObserver = class ResizeObserver {
  observe() {}
  unobserve() {}
  disconnect() {}
}

describe('Header', () => {
  it('renders loading skeleton initially', () => {
    const rootRoute = createRootRoute({
      component: () => <Header />,
    })
    const router = createRouter({ routeTree: rootRoute })

    render(<RouterProvider router={router} />)
  })

  it('renders logo after loading', async () => {
    const rootRoute = createRootRoute({
      component: () => <Header />,
    })
    const router = createRouter({ routeTree: rootRoute })

    render(<RouterProvider router={router} />)

    await waitFor(() => {
      expect(
        screen.getByText(/MEDIDOR DE POTÊNCIA ÓPTICA/i),
      ).toBeInTheDocument()
    })
  })

  it('renders navigation links', async () => {
    const rootRoute = createRootRoute({
      component: () => <Header />,
    })
    const router = createRouter({ routeTree: rootRoute })

    render(<RouterProvider router={router} />)

    await waitFor(() => {
      expect(screen.getByText(/Monitoramento/i)).toBeInTheDocument()
      expect(screen.getByText(/Alarmes e Avisos/i)).toBeInTheDocument()
      expect(screen.getByText(/Calibração/i)).toBeInTheDocument()
      expect(screen.getByText(/Sobre/i)).toBeInTheDocument()
    })
  })
})
