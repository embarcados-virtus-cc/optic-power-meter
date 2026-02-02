import { Store } from '@tanstack/store'

export const loadingStore = new Store(true)

setTimeout(() => {
  loadingStore.setState(false)
}, 500)
