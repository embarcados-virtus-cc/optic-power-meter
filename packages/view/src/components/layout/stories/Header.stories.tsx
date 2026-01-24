import type { Meta, StoryObj } from '@storybook/react'
import { Header } from '../page/Header'
import {
  createRootRoute,
  createRouter,
  RouterProvider,
} from '@tanstack/react-router'
import { useEffect } from 'react'

// Wrapper to provide router context
const DataRouterProvider = ({ children }: { children: React.ReactNode }) => {
  const rootRoute = createRootRoute({
    component: () => <>{children}</>,
  })
  const router = createRouter({ routeTree: rootRoute })
  return <RouterProvider router={router} />
}

const meta = {
  title: 'Layout/Header',
  component: Header,
  parameters: {
    layout: 'fullscreen',
  },
  decorators: [
    (Story) => (
      <DataRouterProvider>
        <Story />
      </DataRouterProvider>
    ),
  ],
  tags: ['autodocs'],
} satisfies Meta<typeof Header>

export default meta
type Story = StoryObj<typeof meta>

export const Default: Story = {}
