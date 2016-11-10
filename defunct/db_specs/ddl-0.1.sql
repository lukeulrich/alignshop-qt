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
    key varchar(32) not null primary key,
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
    foreign_table text,                              -- Database table referred to by this record
    fk_id integer,                                   -- Foreign key to the referred table (non-enforced)
    label text,                                      -- Only valid for non-table entities (e.g. folders/groups)
    lft integer not null,                            -- MPTT field
    rgt integer not null,                            -- MPTT field

    check(type IN ('root', 'project', 'group', 'seqamino', 'seqdna', 'seqrna', 'subseqamino', 'subseqdna', 'subseqrna', 'msaamino', 'msadna', 'msarna', 'primer')),
    check(fk_id > 0),

    check(lft > 0),
    check(rgt > lft)
);


/******************************************************************************
  * Anonymous string tables: these contain purely specific biostring sequence
  *   data
  */
create table astrings
-- Unique, anonymous amino acid sequences
(
    id integer primary key autoincrement,   -- Primary key; sequence generated integer
    aseq_id integer,                        -- Foreign key to SedDepot.aseqs(id)
    digest text unique,                     -- SHA1 hash of sequence for identification purposes; must be unique
    sequence text not null
);

create table dseqs
-- Unique, anonymous DNA sequences
(
    id integer primary key autoincrement,   -- Primary key; sequence generated integer
    hash text unique,                       -- SHA1 hash of sequence for identification purposes; must be unique
    sequence text not null
);

create table rseqs
-- Unique, anonymous RNA sequences
(
    id integer primary key autoincrement,   -- Primary key; sequence generated integer
    hash text unique,                       -- SHA1 hash of sequence for identification purposes; must be unique
    sequence text not null
);



/******************************************************************************
 *****************************************************************************/
create table projects
-- Project associated data
(
    id integer primary key autoincrement,
    name text not null default "Undefined"
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
    id integer primary key autoincrement,   -- Primary key; sequence generated integer
    project_id integer not null,            -- Foreign key to projects(id)
    astring_id integer not null,            -- Foreign key to astrings(id)
    label text not null,                    -- Arbitrary user label

    foreign key(aseq_id) references aseqs(id) on update cascade on delete cascade
);
create index if not exists amino_seqs_astring_id_index on amino_seqs(astring_id);
create index if not exists amino_seqs_project_id_index on amino_seqs(project_id);

create table amino_subseqs
-- Amino acid subsequences
(
    id integer primary key autoincrement,           -- Primary key; sequence generated integer
    amino_seq_id integer not null,                  -- Foreign key to amino_seqs(id)
    label text,                                     -- Arbitrary user label
    start integer not null check (start > 0),       -- Start position (1-based) of subseq within source sequence
    stop integer not null check (stop >= start),    -- Stop position (1-based) of subseq within source sequence
    sequence text not null,                         -- Gapped sequence data for this subseq

    foreign key(amino_seq_id) references amino_seqs(id) on update cascade on delete cascade
);
create index amino_subseqs_amino_seq_id_index on amino_subseqs(amino_seq_id);


-----------------------------------
-->> DNA sequences and subsequences
-----------------------------------
create table dna_seqs
-- DNA sequences and annotations; possibly redundant in both sequence and annotation
(
    id integer primary key autoincrement,   -- Primary key; sequence generated integer
    project_id integer not null,            -- Foreign key to projects(id)
    dseq_id integer not null,               -- Foreign key to dseqs(id)
    label text not null,                    -- Arbitrary user label

    foreign key(dseq_id) references dseqs(id) on update cascade on delete cascade
);
create index if not exists dna_seqs_dseq_id_index on dna_seqs(dseq_id);
create index if not exists dna_seqs_project_id_index on dna_seqs(project_id);

create table dna_subseqs
-- DNA subsequences
(
    id integer primary key autoincrement,           -- Primary key; sequence generated integer
    dna_seq_id integer not null,                    -- Foreign key to dna_seqs(id)
    label text,                                     -- Arbitrary user label
    start integer not null check (start > 0),       -- Start position (1-based) of subseq within source sequence
    stop integer not null check (stop >= start),    -- Stop position (1-based) of subseq within source sequence
    sequence text not null,                         -- Gapped sequence data for this subseq

    foreign key(dna_seq_id) references dna_seqs(id) on update cascade on delete cascade
);
create index dna_subseqs_dna_seq_id_index on dna_subseqs(dna_seq_id);


-----------------------------------
-->> RNA sequences and subsequences
-----------------------------------
create table rna_seqs
-- RNA sequences and annotations; possibly redundant in both sequence and annotation
(
    id integer primary key autoincrement,   -- Primary key; sequence generated integer
    project_id integer not null,            -- Foreign key to projects(id)
    rseq_id integer not null,               -- Foreign key to rseqs(id)
    label text not null,                    -- Arbitrary user label

    foreign key(rseq_id) references rseqs(id) on update cascade on delete cascade
);
create index if not exists rna_seqs_rseq_id_index on rna_seqs(rseq_id);
create index if not exists rna_seqs_project_id_index on rna_seqs(project_id);

create table rna_subseqs
-- RNA subsequences
(
    id integer primary key autoincrement,           -- Primary key; sequence generated integer
    rna_seq_id integer not null,                    -- Foreign key to rna_seqs(id)
    label text,                                     -- Arbitrary user label
    start integer not null check (start > 0),       -- Start position (1-based) of subseq within source sequence
    stop integer not null check (stop >= start),    -- Stop position (1-based) of subseq within source sequence
    sequence text not null,                         -- Gapped sequence data for this subseq

    foreign key(rna_seq_id) references rna_seqs(id) on update cascade on delete cascade
);
create index rna_subseqs_rna_seq_id_index on rna_subseqs(rna_seq_id);

/******************************************************************************
  * Alignments
  */
create table amino_msas
-- Amino-acid multiple sequence alignments
(
    id integer primary key autoincrement,            -- Primary key; sequence generated integer
    project_id integer not null,                     -- Foreign key to projects(id)
    label text not null                              -- Arbitrary user label
);
create index amino_msas_project_id_index on amino_msas(project_id);

create table amino_msas_subseqs
-- The individual subseqs comprising an amino msa
(
    amino_msa_id integer not null,          -- Partial primary key; foreign key to amino_msas(id)
    amino_subseq_id integer not null,       -- Partial primary key; foreign key to amino_subseqs(id)

    primary key(amino_msa_id, amino_subseq_id),
    foreign key(amino_msa_id) references amino_msas(id) on update cascade on delete cascade,
    foreign key(amino_subseq_id) references amino_subseqs(id) on update cascade on delete restrict
);

create table dna_msas
-- DNA multiple sequence alignments
(
    id integer primary key autoincrement,            -- Primary key; sequence generated integer
    project_id integer not null,                     -- Foreign key to projects(id)
    label text not null                              -- Arbitrary user label
);
create index dna_msas_project_id_index on dna_msas(project_id);

create table dna_msas_subseqs
-- The individual subseqs comprising a DNA msa
(
    dna_msa_id integer not null,          -- Partial primary key; foreign key to dna_msas(id)
    dna_subseq_id integer not null,       -- Partial primary key; foreign key to dna_subseqs(id)

    primary key(dna_msa_id, dna_subseq_id),
    foreign key(dna_msa_id) references dna_msas(id) on update cascade on delete cascade,
    foreign key(dna_subseq_id) references dna_subseqs(id) on update cascade on delete restrict
);

create table rna_msas
-- RNA multiple sequence alignments
(
    id integer primary key autoincrement,            -- Primary key; sequence generated integer
    project_id integer not null,                     -- Foreign key to projects(id)
    label text not null                              -- Arbitrary user label
);
create index rna_msas_project_id_index on rna_msas(project_id);

create table rna_msas_subseqs
-- The individual subseqs comprising a RNA msa
(
    rna_msa_id integer not null,          -- Partial primary key; foreign key to rna_msas(id)
    rna_subseq_id integer not null,       -- Partial primary key; foreign key to rna_subseqs(id)

    primary key(rna_msa_id, rna_subseq_id),
    foreign key(rna_msa_id) references rna_msas(id) on update cascade on delete cascade,
    foreign key(rna_subseq_id) references rna_subseqs(id) on update cascade on delete restrict
);
