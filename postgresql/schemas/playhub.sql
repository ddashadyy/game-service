DROP SCHEMA IF EXISTS playhub CASCADE;

CREATE SCHEMA IF NOT EXISTS playhub;

CREATE TABLE IF NOT EXISTS playhub.games (
    id UUID PRIMARY KEY,
    igdb_id TEXT NOT NULL,
    
    name TEXT NOT NULL,
    slug TEXT NOT NULL,
    summary TEXT,
    
    igdb_rating DOUBLE PRECISION,
    playhub_rating DOUBLE PRECISION,
    hypes INTEGER DEFAULT 0,
    
    first_release_date TEXT,
    release_dates TEXT[],
    
    cover_url TEXT,
    artwork_urls TEXT[],
    screenshots TEXT[],
    
    genres TEXT[],
    themes TEXT[],
    platforms TEXT[],

    created_at TIMESTAMP WITHOUT TIME ZONE NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMP WITHOUT TIME ZONE NOT NULL DEFAULT NOW()
);

-- Опционально: индекс для поиска по igdb_id или slug
CREATE INDEX IF NOT EXISTS idx_games_igdb_id ON games(igdb_id);
CREATE INDEX IF NOT EXISTS idx_games_slug ON games(slug);