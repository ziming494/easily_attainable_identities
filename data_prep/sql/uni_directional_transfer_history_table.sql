CREATE
OR REPLACE TABLE kyc_paper.uni_directional_transfer_snapshot_20240531 AS WITH native_eth_transfer AS (
    SELECT
        transaction_hash,
        transaction_index,
        call_type,
        block_number,
        block_timestamp,
        lower(from_address) address,
        lower(to_address) counterparty,
        'send' operation,
        value / 1e18 amount,
        trace_id
    FROM
        `bigquery-public-data.crypto_ethereum.traces`
    WHERE
        value * STATUS > 0
        AND coalesce(call_type, 'call') = 'call'
        AND date(block_timestamp) <= '2024-05-31'
),
native_eth AS (
    SELECT
        address,
        counterparty,
        sum(abs(amount) * b.price) AS eth_transfer_usd
    FROM
        native_eth_transfer a
        LEFT JOIN `circle-ds-pipelines.kyc_paper.cgc_daily_eth_close` b ON date(a.block_timestamp) = date(b.snapped_at)
    WHERE
        date(a.block_timestamp) <= date('2024-05-31')
    GROUP BY
        1,
        2
),
wrapped_eth AS (
    SELECT
        address,
        counterparty,
        sum(abs(amount) * b.price) AS weth_transfer_usd
    FROM
        `circle-ds-pipelines.ethereum.fct_token_transfers` a
        LEFT JOIN `circle-ds-pipelines.kyc_paper.cgc_daily_eth_close` b ON date(a.block_timestamp) = date(b.snapped_at)
    WHERE
        token = 'WETH'
        AND operation = 'send'
        AND date(a.block_timestamp) <= date('2024-05-31')
    GROUP BY
        1,
        2
),
stables AS (
    SELECT
        address,
        counterparty,
        sum(abs(amount)) AS stables_transfer_usd
    FROM
        `circle-ds-pipelines.ethereum.fct_token_transfers` a
    WHERE
        token IN ('USDC', 'USDT')
        AND operation = 'send'
        AND date(a.block_timestamp) <= date('2024-05-31')
    GROUP BY
        1,
        2
),
sc AS (
    SELECT
        DISTINCT address
    FROM
        `bigquery-public-data.crypto_ethereum.contracts`
),
gnosis AS (
    SELECT
        DISTINCT address
    FROM
        `circle-ds-pipelines.kyc_paper.gnosis_safe`
)
SELECT
    coalesce(a.address, b.address, c.address) AS address,
    coalesce(a.counterparty, b.counterparty, c.counterparty) AS counterparty,
    coalesce(a.eth_transfer_usd, 0) + coalesce(b.weth_transfer_usd, 0) + coalesce(c.stables_transfer_usd, 0) AS total_transfer_usd
FROM
    native_eth a FULL
    OUTER JOIN wrapped_eth b ON a.address = b.address
    AND a.counterparty = b.counterparty FULL
    OUTER JOIN stables c ON coalesce(a.address, b.address) = c.address
    AND coalesce(a.counterparty, b.counterparty) = c.counterparty
    LEFT JOIN sc d1 ON coalesce(a.address, b.address, c.address) = d1.address
    LEFT JOIN sc d2 ON coalesce(a.counterparty, b.counterparty, c.counterparty) = d2.address
    LEFT JOIN gnosis g1 ON coalesce(a.address, b.address, c.address) = g1.address
    LEFT JOIN gnosis g2 ON coalesce(a.counterparty, b.counterparty, c.counterparty) = g2.address
WHERE
    (
        d1.address IS NULL
        OR g1.address IS NOT NULL
    )
    AND (
        d2.address IS NULL
        OR g2.address IS NOT NULL
    )
    AND coalesce(a.address, b.address, c.address) IS NOT NULL
    AND coalesce(a.address, b.address, c.address) != '0x0000000000000000000000000000000000000000'
    AND coalesce(a.counterparty, b.counterparty, c.counterparty) IS NOT NULL
    AND coalesce(a.counterparty, b.counterparty, c.counterparty) != '0x0000000000000000000000000000000000000000'
ORDER BY
    1,
    2