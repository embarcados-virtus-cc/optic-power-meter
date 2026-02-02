import type { Meta, StoryObj } from '@storybook/react'
import { About } from '../page/Sidebar'
import { useState } from 'react'

const meta = {
  title: 'About',
  component: About,
  parameters: {
    layout: 'centered',
  },
  tags: ['autodocs'],
} satisfies Meta<typeof About>

export default meta
type Story = StoryObj<typeof meta>

export const Default: Story = {
  args: {
    open: true,
    onOpenChange: () => {},
  },
  render: (args) => {
    const [isOpen, setIsOpen] = useState(args.open)
    return <About open={isOpen} onOpenChange={setIsOpen} />
  },
}
