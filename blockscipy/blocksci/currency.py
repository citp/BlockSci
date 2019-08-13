import datetime
import requests
import pandas as pd

try:
    from IPython.core.display import display
    def _print_coindesk_info():
        display('Exchange rates are provided by CoinDesk (https://www.coindesk.com/price/).')
except ImportError:
    def _print_coindesk_info():
        print('Exchange rates are provided by CoinDesk (https://www.coindesk.com/price/).')


class CurrencyConverter(object):
    """
    Imports Bitcoin exchange rates in a variety of currencies using the Coindesk API available at https://www.coindesk.com/price/.
    """

    min_start = pd.to_datetime('2009-01-03').date()
    max_end = datetime.date.today() - datetime.timedelta(days=1)
    # the API has data starting at 2010-07-19
    COINDESK_START = pd.to_datetime('2010-07-19').date()

    def __init__(self, currency='USD', start=min_start, end=max_end):
        _print_coindesk_info()

        self.currency = currency

        self.start = self.validate_date(start)
        self.end = self.validate_date(end)

        if self.start > self.end:
            raise ValueError("End date cannot be ahead of start date.")

        self.supported_currencies = self._get_supported_currencies()
        if currency not in self.supported_currencies:
            raise ValueError("Currency {} is not supported. Please use one of the following options: {}.".format(currency, self.supported_currencies))

        self.data = self._get_data()

    def _get_supported_currencies(self):
        r = requests.get('https://api.coindesk.com/v1/bpi/supported-currencies.json')
        r.raise_for_status()
        return [x['currency'] for x in r.json()]

    def _get_data(self):
        base_url = 'https://api.coindesk.com/v1/bpi/historical/close.json'
        r = requests.get('{}?index=USD&currency={}&start={}&end={}'.format(base_url, self.currency, max(self.COINDESK_START, self.start), max(self.COINDESK_START, self.end)))
        r.raise_for_status()
        return r.json()['bpi']

    def validate_date(self, date):
        newdate = pd.to_datetime(date).date()
        if newdate < self.min_start or newdate > self.max_end:
            raise ValueError("Date must be between {} and {}.".format(self.min_start, self.max_end))
        return newdate

    def exchangerate(self, date):
        if date < self.COINDESK_START:
            return 0
        return self.data[str(date)]

    def btc_to_currency(self, value, date):
        date = self.validate_date(date)
        return value * self.exchangerate(date)

    def _btc_to_currency(self, value, date):
        return value * self.exchangerate(date)

    def bitcoin_to_currency(self, value, date):
        return self.btc_to_currency(value, date)

    def _satoshi_to_currency(self, value, date):
        return self._btc_to_currency(value / 1e8, date)

    def satoshi_to_currency(self, value, date):
        return self.btc_to_currency(value / 1e8, date)

    def satoshi_to_currency_df(self, df, chain, columns=None):
        if columns is None:
            columns = df.columns
        def convert_row(row):
            date = row["index"]
            if hasattr(date, 'date'):
                date = date.date()
            rate = self.exchangerate(date) / 1e8
            for column in columns:
                if column in row:
                    row[column] = rate * row[column]
            return row
        df["index"] = df.index
        index_type = str(df["index"].dtype)
        if index_type == "int64":
            df["index"] = pd.Series(df["index"]).apply(lambda x: chain[x].time.date())
        df = df.apply(convert_row, axis=1)
        del df["index"]
        return df

    def currency_to_btc(self, value, date):
        date = self.validate_date(date)
        if date < self.COINDESK_START:
            return None
        return value / self.exchangerate(date)

    def currency_to_bitcoin(self, value, date):
        return self.currency_to_btc(value, date)

    def currency_to_satoshi(self, value, date):
        return self.currency_to_btc(value, date) * 1e8
