import { integer, sqliteTable, text, index } from 'drizzle-orm/sqlite-core';
export const guests = sqliteTable('guests', {
 tokenHash:text('token_hash').primaryKey(),authorId:text('author_id').notNull().unique(),
 nick:text('nick').notNull(),createdAt:integer('created_at').notNull(),lastPost:integer('last_post').notNull().default(0)
});
export const messages = sqliteTable('messages', {
 id:integer('id').primaryKey({autoIncrement:true}),messageKey:text('message_key').notNull().unique(),
 authorId:text('author_id').notNull(),nick:text('nick').notNull(),body:text('body').notNull(),createdAt:integer('created_at').notNull()
}, t=>[index('messages_author').on(t.authorId)]);
export const rateLimits = sqliteTable('rate_limits', {
 key:text('key').primaryKey(),count:integer('count').notNull(),windowStart:integer('window_start').notNull()
});
