import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import { api } from './api'
import { mocks } from '../mocks'

describe('API Client Mock Fallback', () => {
    beforeEach(() => {
        vi.stubGlobal('fetch', vi.fn())
    })

    afterEach(() => {
        vi.unstubAllGlobals()
        vi.unstubAllEnvs()
    })

    it('should return mock data for "current" endpoint when fetch fails in DEV', async () => {
        // Simulate DEV environment
        vi.stubEnv('DEV', true)

        // Simulate fetch failure
        vi.mocked(fetch).mockRejectedValueOnce(new Error('Network Error'))

        const data = await api.current()
        expect(data).toEqual(mocks.current)
    })

    it('should return mock data for "history" endpoint when fetch fails in DEV', async () => {
        // Simulate DEV environment
        vi.stubEnv('DEV', true)

        // Simulate fetch failure
        vi.mocked(fetch).mockRejectedValueOnce(new Error('Network Error'))

        const data = await api.history()
        expect(data).toEqual(mocks.history)
    })

    it('should throw error when fetch fails in PROD (not DEV)', async () => {
        // Simulate PROD environment (DEV = false)
        vi.stubEnv('DEV', false)

        // Simulate fetch failure
        const error = new Error('Network Error')
        vi.mocked(fetch).mockRejectedValueOnce(error)

        await expect(api.current()).rejects.toThrow('Network Error')
    })
})
