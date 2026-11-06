CREATE TABLE `guests` (
	`token_hash` text PRIMARY KEY NOT NULL,
	`author_id` text NOT NULL,
	`nick` text NOT NULL,
	`created_at` integer NOT NULL,
	`last_post` integer DEFAULT 0 NOT NULL
);
--> statement-breakpoint
CREATE UNIQUE INDEX `guests_author_id_unique` ON `guests` (`author_id`);--> statement-breakpoint
CREATE TABLE `messages` (
	`id` integer PRIMARY KEY AUTOINCREMENT NOT NULL,
	`message_key` text NOT NULL,
	`author_id` text NOT NULL,
	`nick` text NOT NULL,
	`body` text NOT NULL,
	`created_at` integer NOT NULL
);
--> statement-breakpoint
CREATE UNIQUE INDEX `messages_message_key_unique` ON `messages` (`message_key`);--> statement-breakpoint
CREATE INDEX `messages_author` ON `messages` (`author_id`);--> statement-breakpoint
CREATE TABLE `rate_limits` (
	`key` text PRIMARY KEY NOT NULL,
	`count` integer NOT NULL,
	`window_start` integer NOT NULL
);
