/* DDL style

-- All text except comments must be in lowercase

*/

pragma foreign_keys = ON;


/******************************************************************************
  * Metadata about this datafile is stored within the database itself.
  * Simply key => value association of data
  * Standard keys are as follows:
  *
  * author
  * created
  * modified
  * alignshop_version => X.Y
  * schema_version => X.Y
  *
  */
create table __info
-- Metadata describing this document file
(
    name text not null primary key,
    value text
);



/******************************************************************************
  * The data is organized into a fancy tree structure which is also stored in
  * the database using a Modified Preorder Tree Traversal approach.
  *
  * Example tree structure
  *
  *    ID  ParentID    TABLE       FK_ID   NAME        LFT     RGT
  *    1   NULL        NULL        NULL    Root        1       20
  *    2   1           projects    1       NULL        2       3
  *    3   1           projects    2       NULL        4       17
  *    4   3           NULL        NULL    Group-1     5       6
  *    5   3           NULL        NULL    Group-1.1   7       16
  *    6   5           amino_seqs  1       NULL        8       9
  *    7   5           amino_seqs  2       NULL        10      11
  *    8   5           amino_seqs  3       NULL        12      13
  *    9   5           NULL        NULL    Subgroup    14      15
  *    10  1           projects    3       NULL        18      19
  *
  * Since one database file represents one entire user file and spans N projects, only a single MPTT is stored
  * herein. Did not name the table MPTT data tree (or similar) to avoid having to rename the table should we elect
  * to use an alternative strategy for storing the tree.
  *
  * Another advantage of storing the tree in MPTT form: power users can easily query it directly with SQL given they
  * are familiar with interacting with MPTT trees.
  */
create table data_tree
-- MPTT compatible tree structure
(
    id integer primary key autoincrement,            -- Primary key; sequence generated integer
    type text not null,                              -- Node type. Constrained to specific types (see check below)
    fk_id integer,                                   -- Foreign key to the referred table (non-enforced)
    label text,                                      -- Only valid for non-table entities (e.g. folders/groups)
    lft integer not null,                            -- MPTT field
    rgt integer not null,                            -- MPTT field

    check(type IN ('root', 'group', 'seqamino', 'seqdna', 'seqrna', 'subseqamino', 'subseqdna', 'subseqrna', 'msaamino', 'msadna', 'msarna', 'primer')),
    check(fk_id > 0 || (type IN ('root','group') && fk_id IS NULL),

    check(lft > 0),
    check(rgt > lft)
);


/******************************************************************************
  * Anonymous string tables: these contain the raw biostring sequence data and potentially
  * its external SeqDepot identifier.
  */
create table astrings
-- Unique, anonymous amino acid sequences
(
    id integer primary key autoincrement,   -- Primary key; sequence generated integer
    aseq_id integer,                        -- Foreign key to SedDepot.aseqs(id)
    digest text unique,                     -- SHA1 hash of sequence for identification purposes; must be unique
    sequence text not null,                 -- Raw, normalized sequence data

    check(length(sequence) > 0)
);

create table dstrings
-- Unique, anonymous DNA sequences
(
    id integer primary key autoincrement,   -- Primary key; sequence generated integer
    dseq_id integer,                        -- Foreign key to SedDepot.dseqs(id)
    digest text unique,                     -- SHA1 hash of sequence for identification purposes; must be unique
    sequence text not null,                 -- Raw, normalized sequence data

    check(length(sequence) > 0)
);

create table rstrings
-- Unique, anonymous RNA sequences
(
    id integer primary key autoincrement,   -- Primary key; sequence generated integer
    rseq_id integer,                        -- Foreign key to SedDepot.rseqs(id)
    digest text unique,                     -- SHA1 hash of sequence for identification purposes; must be unique
    sequence text not null,                 -- Raw, normalized sequence data

    check(length(sequence) > 0)
);



/******************************************************************************
  * Actual sequence and their annotations are stored in the following tables
  *   which correspond to the above anonymous string tables via their data
  *   type (e.g. Amino, DNA, RNA)
  */

------------------------------------------
-->> Amino acid sequences and subsequences
------------------------------------------
create table amino_seqs
-- Amino acid sequences and annotations; possibly redundant in both sequence and annotation
(
    id integer primary key autoincrement,               -- Primary key; sequence generated integer
    astring_id integer not null,                        -- Foreign key to astrings(id)
    label text not null,                                -- Arbitrary user label
    source text,                                        -- Sequence source (e.g. organism, virus, etc.)
    protein text,                                       -- Name of protein (if applicable)
    created text not null default current_timestamp,    -- When this record was created

    foreign key(astring_id) references astrings(id) on update cascade on delete cascade
);
create index if not exists amino_seqs_astring_id_index on amino_seqs(astring_id);

create table amino_subseqs
-- Amino acid subsequences
(
    id integer primary key autoincrement,               -- Primary key; sequence generated integer
    amino_seq_id integer not null,                      -- Foreign key to amino_seqs(id)
    label text,                                         -- Arbitrary user label
    start integer not null,                             -- Start position (1-based) of subseq within source sequence
    stop integer not null,                              -- Stop position (1-based) of subseq within source sequence
    sequence text not null,                             -- Gapped sequence data for this subseq
    notes text,                                         -- User-annotation
    created text not null default current_timestamp,    -- When this record was created

    check (start > 0),
    check (stop >= start),

    foreign key(amino_seq_id) references amino_seqs(id) on update cascade on delete cascade
);
create index amino_subseqs_amino_seq_id_index on amino_subseqs(amino_seq_id);

create table amino_msas
-- Amino acid multiple sequence alignments
(
    id integer primary key autoincrement,               -- Primary key; sequence generated integer
    name text not null,                                 -- Custom alignment name
    description text,                                   -- Free-form description
    created text not null default current_timestamp     -- When this record was created
);

create table amino_msas_subseqs
-- Amino acid subsequences belonging to an alignment
(
    id integer primary key autoincrement,               -- Primary key; sequence generated integer
    amino_msa_id integer not null,                      -- Foreign key to amino_msas(id)
    amino_subseq_id integer not null,                   -- Foreign key to amino_subseqs(id)
    position integer not null,                          -- Order of this subseq within the MSA

    unique(amino_msa_id, amino_subseq_id),
    foreign key(amino_msa_id) references amino_msas(id) on update cascade on delete cascade,
    foreign key(amino_subseq_id) references amino_subseqs(id) on update cascade on delete cascade
);



-----------------------------------
-->> DNA sequences and subsequences
-----------------------------------
create table dna_seqs
-- DNA sequences and annotations; possibly redundant in both sequence and annotation
(
    id integer primary key autoincrement,               -- Primary key; sequence generated integer
    dstring_id integer not null,                        -- Foreign key to dstrings(id)
    label text not null,                                -- Arbitrary user label
    source text,                                        -- Sequence source (e.g. organism, virus, etc.)
    gene text,                                          -- Name of gene (if applicable)
    created text not null default current_timestamp,    -- When this record was created

    foreign key(dstring_id) references dstrings(id) on update cascade on delete cascade
);
create index if not exists dna_seqs_dstring_id_index on dna_seqs(dstring_id);

create table dna_subseqs
-- DNA subsequences
(
    id integer primary key autoincrement,               -- Primary key; sequence generated integer
    dna_seq_id integer not null,                        -- Foreign key to dna_seqs(id)
    label text,                                         -- Arbitrary user label
    start integer not null,                             -- Start position (1-based) of subseq within source sequence
    stop integer not null,                              -- Stop position (1-based) of subseq within source sequence
    sequence text not null,                             -- Gapped sequence data for this subseq
    notes text,                                         -- User-annotation
    created text not null default current_timestamp,    -- When this record was created

    check (start > 0),
    check (stop >= start),

    foreign key(dna_seq_id) references dna_seqs(id) on update cascade on delete cascade
);
create index dna_subseqs_dna_seq_id_index on dna_subseqs(dna_seq_id);

create table dna_msas
-- DNA multiple sequence alignments
(
    id integer primary key autoincrement,               -- Primary key; sequence generated integer
    name text not null,                                 -- Custom alignment name
    description text,                                   -- Free-form description
    created text not null default current_timestamp     -- When this record was created
);

create table dna_msas_subseqs
-- DNA subsequences belonging to an alignment
(
    id integer primary key autoincrement,               -- Primary key; sequence generated integer
    dna_msa_id integer not null,                        -- Foreign key to dna_msas(id)
    dna_subseq_id integer not null,                     -- Foreign key to dna_subseqs(id)
    position integer not null,                          -- Order of this subseq within the MSA

    unique(dna_msa_id, dna_subseq_id),
    foreign key(dna_msa_id) references dna_msas(id) on update cascade on delete cascade,
    foreign key(dna_subseq_id) references dna_subseqs(id) on update cascade on delete cascade
);


-----------------------------------
-->> RNA sequences and subsequences
-----------------------------------
create table rna_seqs
-- RNA sequences and annotations; possibly redundant in both sequence and annotation
(
    id integer primary key autoincrement,               -- Primary key; sequence generated integer
    rstring_id integer not null,                        -- Foreign key to rstrings(id)
    label text not null,                                -- Arbitrary user label
    source text,                                        -- Sequence source (e.g. organism, virus, etc.)
    gene text,                                          -- Name of gene (if applicable)
    created text not null default current_timestamp,    -- When this record was created

    foreign key(rstring_id) references rstrings(id) on update cascade on delete cascade
);
create index if not exists rna_seqs_rstring_id_index on rna_seqs(rstring_id);

create table rna_subseqs
-- RNA subsequences
(
    id integer primary key autoincrement,               -- Primary key; sequence generated integer
    rna_seq_id integer not null,                        -- Foreign key to rna_seqs(id)
    label text,                                         -- Arbitrary user label
    start integer not null,                             -- Start position (1-based) of subseq within source sequence
    stop integer not null,                              -- Stop position (1-based) of subseq within source sequence
    sequence text not null,                             -- Gapped sequence data for this subseq
    notes text,                                         -- User-annotation
    created text not null default current_timestamp,    -- When this record was created

    check (start > 0),
    check (stop >= start),

    foreign key(rna_seq_id) references rna_seqs(id) on update cascade on delete cascade
);
create index rna_subseqs_rna_seq_id_index on rna_subseqs(rna_seq_id);

create table rna_msas
-- RNA multiple sequence alignments
(
    id integer primary key autoincrement,               -- Primary key; sequence generated integer
    name text not null,                                 -- Custom alignment name
    description text,                                   -- Free-form description
    created text not null default current_timestamp     -- When this record was created
);

create table rna_msas_subseqs
-- RNA subsequences belonging to an alignment
(
    id integer primary key autoincrement,               -- Primary key; sequence generated integer
    rna_msa_id integer not null,                        -- Foreign key to rna_msas(id)
    rna_subseq_id integer not null,                     -- Foreign key to rna_subseqs(id)
    position integer not null,                          -- Order of this subseq within the MSA

    unique(rna_msa_id, rna_subseq_id),
    foreign key(rna_msa_id) references rna_msas(id) on update cascade on delete cascade,
    foreign key(rna_subseq_id) references rna_subseqs(id) on update cascade on delete cascade
);
