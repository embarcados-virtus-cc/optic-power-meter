import type { Meta, StoryObj } from '@storybook/react'
import { Info } from '../page/Info'

const meta = {
  title: 'Dashboard/Info',
  component: Info,
  parameters: {
    layout: 'padded',
  },
  tags: ['autodocs'],
} satisfies Meta<typeof Info>

export default meta
type Story = StoryObj<typeof meta>

export const Default: Story = {
  args: {
    isLoading: false,
  },
}

export const Loading: Story = {
  args: {
    isLoading: true,
  },
}
