DROP SCHEMA IF EXISTS playhub CASCADE;

CREATE SCHEMA IF NOT EXISTS playhub;

CREATE TABLE IF NOT EXISTS playhub.games (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    igdb_id TEXT NOT NULL UNIQUE ,
    
    name TEXT NOT NULL,
    slug TEXT NOT NULL UNIQUE , 
    summary TEXT,
    
    igdb_rating INTEGER DEFAULT 0,
    playhub_rating INTEGER DEFAULT 0,
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

CREATE INDEX IF NOT EXISTS idx_games_igdb_id ON playhub.games(igdb_id);
CREATE UNIQUE INDEX IF NOT EXISTS idx_games_slug ON playhub.games(slug);
