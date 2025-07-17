/*
 *   Copyright (c) 2024 Cameron Dudd
 *   All rights reserved.
 */

PRAGMA foreign_keys = ON;
PRAGMA synchronous = NORMAL;
PRAGMA cache_size = 10000;
PRAGMA temp_store = MEMORY;
PRAGMA case_sensitive_like = ON;

CREATE TABLE IF NOT EXISTS machines (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    brand TEXT NOT NULL,
    model TEXT NOT NULL,
    supports_pre_infusion INTEGER NOT NULL CHECK (supports_pre_infusion IN (0, 1))
);

CREATE TABLE IF NOT EXISTS grinders (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    brand TEXT NOT NULL,
    automatic INTEGER NOT NULL CHECK (automatic IN (0, 1))
);

CREATE TABLE IF NOT EXISTS grind_settings (
    id INTEGER PRIMARY KEY,
    grinder_id INTEGER NOT NULL,
    grinder_config TEXT CHECK (json_valid(grinder_config)),
    FOREIGN KEY (grinder_id) REFERENCES grinders(id)
);

CREATE TABLE IF NOT EXISTS brewing_methods (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS beans (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    roaster TEXT,
    origin TEXT,
    roast_time INTEGER CHECK (roast_time >= 0),  -- Unix time seconds
    roast_level INTEGER CHECK (roast_level BETWEEN 0 AND 5),
    process TEXT,
    notes TEXT
);

CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY,
    username TEXT NOT NULL,
    name TEXT,
    email TEXT
);

CREATE TABLE IF NOT EXISTS locations (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    lat FLOAT,
    lon FLOAT
);

CREATE TABLE IF NOT EXISTS brew_sessions (
    id INTEGER PRIMARY KEY,
    dose_grams FLOAT,
    yield_grams FLOAT,
    pressure_bar FLOAT,
    brew_time INTEGER NOT NULL CHECK (brew_time >= 0),  -- Unix time seconds
    rating INTEGER NOT NULL CHECK (rating BETWEEN 0 AND 10),
    bean_id INTEGER NOT NULL,
    grind_setting_id INTEGER NOT NULL,
    machine_id INTEGER NOT NULL,
    FOREIGN KEY (bean_id) REFERENCES beans(id),
    FOREIGN KEY (grind_setting_id) REFERENCES grind_settings(id),
    FOREIGN KEY (machine_id) REFERENCES machines(id)
);
