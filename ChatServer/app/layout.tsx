import type { Metadata } from "next";
import "./globals.css";

export const metadata: Metadata = {
  title: "Part Time Gang",
  description: "La sala compartida de Joseador. Entra con tu apodo y conecta con la comunidad.",
  icons: {
    icon: "/favicon.svg",
    shortcut: "/favicon.svg",
  },
};

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html lang="es">
      <body className="antialiased">{children}</body>
    </html>
  );
}
