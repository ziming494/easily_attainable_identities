/* ------ Preparations ------*/
CREATE
OR REPLACE TABLE kyc_paper.eai_no_dusting AS WITH sc AS (
    /* Get a list of smart contract addresses in order to consider EOA only transactions */
    SELECT
        DISTINCT address
    FROM
        `bigquery-public-data.crypto_ethereum.contracts`
),
eth_transfer AS (
    /* Identify all wallet to wallet ETH transfers */
    SELECT
        DISTINCT from_address,
        to_address
    FROM
        `bigquery-public-data.crypto_ethereum.traces` t
        LEFT JOIN sc d1 ON t.from_address = d1.address
        LEFT JOIN sc d2 ON t.to_address = d2.address
    WHERE
        /*Success ETH transfer*/
        t.value * t.STATUS > 0
        AND coalesce(t.call_type, 'call') = 'call'
        AND t.input = '0x'
        /* Neither sender / receiver is smart contract */
        AND d1.address IS NULL
        AND d2.address IS NULL
        /* Date */
        AND date(t.block_timestamp) <= '2024-05-31'
),
/* ------ Category 1: Exchange and VASPs Address ------ */
exchange_and_vasps AS (
    SELECT
        DISTINCT address
    FROM
        kyc_paper.exchange_address
),
/* ------ Category 2: addresses that directly receive fund from hot wallets ------ */
erc20_user_address AS (
    SELECT
        /* Mark receiver as kyc'd user address */
        DISTINCT to_address address
    FROM
        `bigquery-public-data.crypto_ethereum.token_transfers` a
        /* Join on from_address */
        LEFT JOIN exchange_and_vasps b ON a.from_address = b.address
        LEFT JOIN sc d1 ON a.from_address = d1.address
        LEFT JOIN sc d2 ON a.to_address = d2.address
    WHERE
        date(a.block_timestamp) <= '2024-05-31'
        /* Sender is an exchange */
        AND b.address IS NOT NULL
        AND lower(a.token_address) IN (
            /* USDC */
            '0xa0b86991c6218b36c1d19d4a2e9eb0ce3606eb48',
            /* USDT */
            '0xdac17f958d2ee523a2206206994597c13d831ec7',
            /* WETH */
            '0xc02aaa39b223fe8d0a0e5c4f27ead9083c756cc2'
        )
        /* Neither sender / receiver is smart contract */
        AND d1.address IS NULL
        AND d2.address IS NULL
),
eth_user_address AS (
    SELECT
        /* Mark receiver as kyc'd user address */
        DISTINCT a.to_address address
    FROM
        eth_transfer a
        LEFT JOIN exchange_and_vasps b ON a.from_address = b.address
    WHERE
        /* Sender is an exchange */
        b.address IS NOT NULL
),
summary AS(
    /* ------ Category 1 ------ */
    SELECT
        DISTINCT address,
        'Exchange Wallets' category,
        1 AS cat_rank
    FROM
        exchange_and_vasps
    UNION
    DISTINCT
    /* ------ Category 2 ------ */
    SELECT
        DISTINCT address,
        'User Addresses' category,
        2 AS cat_rank
    FROM
        erc20_user_address
    UNION
    DISTINCT
    SELECT
        DISTINCT address,
        'User Addresses' category,
        3 AS cat_rank
    FROM
        eth_user_address
)
SELECT
    address,
    ARRAY_AGG(
        category
        ORDER BY
            cat_rank ASC
        LIMIT
            1
    ) [OFFSET(0)] category
FROM
    summary
GROUP BY
    address