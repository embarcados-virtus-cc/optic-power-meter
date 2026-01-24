export const CardComponent = ({ className = '', ...props }) => (
  <div
    className={`rounded-lg bg-zinc-900 text-slate-300 shadow-sm ${className}`}
    {...props}
  />
)

export const CardHeaderComponent = ({ className = '', ...props }) => (
  <div className={`flex flex-col space-y-1.5 p-6 ${className}`} {...props} />
)

export const CardTitleComponent = ({ className = '', ...props }) => (
  <h3
    className={`text-base font-semibold leading-none tracking-tight text-slate-300 ${className}`}
    {...props}
  />
)

export const CardContentComponent = ({ className = '', ...props }) => (
  <div className={`p-6 pt-0 ${className}`} {...props} />
)

export const CardFooterComponent = ({ className = '', ...props }) => (
  <div className={`p-6 pt-0 ${className}`} {...props} />
)
