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

CREATE TABLE IF NOT EXISTS grinder_settings (
    id INTEGER PRIMARY KEY,
    grinder_id INTEGER NOT NULL,
    grinder_config TEXT CHECK (json_valid(grinder_config)),
    FOREIGN KEY (grinder_id) REFERENCES grinders(id)
);

CREATE TABLE IF NOT EXISTS brewing_methods (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY,
    username TEXT NOT NULL,
    password TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS vendors (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    website TEXT,
    phone TEXT
);

CREATE TABLE IF NOT EXISTS beans (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    origin TEXT,
    vendor_id INTEGER,
    roast_time INTEGER CHECK (roast_time >= 0),  -- Unix time seconds
    roast_level INTEGER CHECK (roast_level BETWEEN 0 AND 5),
    process TEXT,  -- washed? Maybe switch to boolean
    notes TEXT,
    FOREIGN KEY (vendor_id) REFERENCES vendors(id)
);

CREATE TABLE IF NOT EXISTS vendor_locations (
    id INTEGER PRIMARY KEY,
    vendor_id INTEGER NOT NULL,
    lat FLOAT,
    lon FLOAT,
    FOREIGN KEY (vendor_id) REFERENCES vendors(id)
);

CREATE TABLE IF NOT EXISTS cafes (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS cafe_locations (
    id INTEGER PRIMARY KEY,
    cafe_id INTEGER NOT NULL,
    lat FLOAT,
    lon FLOAT,
    FOREIGN KEY (cafe_id) REFERENCES cafes(id)
);

CREATE TABLE IF NOT EXISTS brew_sessions (
    id INTEGER PRIMARY KEY,
    dose_grams FLOAT,
    yield_grams FLOAT,
    pressure_bar FLOAT,
    brew_time INTEGER CHECK (brew_time >= 0),
    rating INTEGER NOT NULL CHECK (rating BETWEEN 0 AND 10),
    date INTEGER NOT NULL CHECK (date >= 0),  -- Unix time seconds
    bean_id INTEGER,
    grind_setting_id INTEGER,
    brewing_method_id INTEGER,
    user_id INTEGER,
    machine_id INTEGER,
    cafe_location_id INTEGER,
    notes TEXT,
    FOREIGN KEY (bean_id) REFERENCES beans(id),
    FOREIGN KEY (grind_setting_id) REFERENCES grind_settings(id),
    FOREIGN KEY (brewing_method_id) REFERENCES brewing_methods(id),
    FOREIGN KEY (user_id) REFERENCES users(id),
    FOREIGN KEY (machine_id) REFERENCES machines(id),
    FOREIGN KEY (cafe_location_id) REFERENCES cafe_locations(id)
);
