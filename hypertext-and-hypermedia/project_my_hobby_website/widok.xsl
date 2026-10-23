<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="html" version="4.0" encoding="UTF-8" indent="yes"/>

    <xsl:variable name="liczbaZawodnikow" select="count(//Zawodnik)"/>

    <xsl:variable name="stopkaContent">
        <p>Projekt zaliczeniowy. Wszystkie prawa zastrzeżone.</p>
        <p>Rok akademicki: 2024/2025</p>
    </xsl:variable>

    <xsl:template match="/">
        <html lang="pl">
            <head>
                <meta charset="UTF-8"/>
                <title><xsl:value-of select="PortalLekkoatletyczny/Naglowek/Temat"/></title>
                <link rel="stylesheet" href="styl.css"/>
            </head>
            <body>
                <div class="kontener">
                    <xsl:apply-templates select="PortalLekkoatletyczny/Naglowek"/>
                    <xsl:apply-templates select="PortalLekkoatletyczny/ZwiazekSportowy"/>

                    <hr/>

                    <h2>Tabela Zawodników (Posortowana)</h2>
                    <table>
                        <thead>
                            <tr>
                                <th>Lp.</th>
                                <th>Imię i Nazwisko</th>
                                <th>Klub</th>
                                <th>Status</th>
                                <th>Waga (kg)</th>
                            </tr>
                        </thead>
                        <tbody>
                            <xsl:for-each select="//Zawodnik">
                                <xsl:sort select="DaneOsobowe/Nazwisko" order="ascending"/>
                                <tr>
                                    <td>
                                        <xsl:number value="position()" format="1."/>
                                    </td>
                                    <td>
                                        <xsl:value-of select="DaneOsobowe/Imie"/>
                                        <xsl:text> </xsl:text>
                                        <xsl:value-of select="DaneOsobowe/Nazwisko"/>
                                    </td>
                                    <td><xsl:value-of select="../../Nazwa"/></td>
                                    <td>
                                        <xsl:apply-templates select="@status"/>
                                    </td>
                                    <td>
                                        <xsl:value-of select="format-number(ParametryFizyczne/Waga, '#0.0')"/>
                                    </td>
                                </tr>
                            </xsl:for-each>
                        </tbody>
                    </table>

                    <div class="statystyki">
                        <p>Łącznie zawodników w bazie: <strong><xsl:value-of select="$liczbaZawodnikow"/></strong></p>
                        <p>Suma cen biletów (tylko normalne): 
                           <xsl:value-of select="sum(//Bilet[@typ='normalny']/@cena)"/> PLN
                        </p>
                    </div>

                    <hr/>
                    <h2>Szczegółowe Profile</h2>
                    <xsl:apply-templates select="PortalLekkoatletyczny/Kluby/Klub"/>

                    <xsl:call-template name="stopka"/>
                </div>
            </body>
        </html>
    </xsl:template>

    <xsl:template match="Naglowek">
        <header>
            <h1><xsl:value-of select="Temat"/></h1>
            <h3>Autor: <xsl:value-of select="Autor/Imie"/> <xsl:value-of select="Autor/Nazwisko"/></h3>
            <p><i><xsl:value-of select="Opis"/></i></p>
        </header>
    </xsl:template>

    <xsl:template match="ZwiazekSportowy">
        <div class="info-zwiazek">
            <strong>Patronat: </strong> 
            <a href="{StronaOficjalna}" target="_blank">
                <xsl:value-of select="NazwaPelna"/>
            </a>
            <br/>
            Adres: <xsl:value-of select="Adres/Miasto"/>, <xsl:value-of select="Adres/Ulica"/>
        </div>
    </xsl:template>

    <xsl:template match="Klub">
        <div class="sekcja-klubu">
            <h3>Klub: <xsl:value-of select="Nazwa"/> (Zał. <xsl:value-of select="DataZalozenia"/>)</h3>
            <xsl:apply-templates select="Sekcja"/>
        </div>
    </xsl:template>

    <xsl:template match="Sekcja">
        <div class="sekcja-sportowa">
            <h4>Sekcja: <xsl:value-of select="@nazwa"/></h4>
            <xsl:apply-templates select="Zawodnicy/Zawodnik"/>
        </div>
    </xsl:template>

    <xsl:template match="Zawodnik">
        <div class="profil-zawodnika">
            <h4>
                Profil #<xsl:number level="any" count="Zawodnik" format="I"/>: 
                <xsl:value-of select="DaneOsobowe/Imie"/> <xsl:value-of select="DaneOsobowe/Nazwisko"/>
            </h4>
            
            <xsl:apply-templates select="Multimedia"/>
            
            <p><strong>Biografia: </strong> <xsl:value-of select="Biografia"/></p>
            
            <xsl:if test="string-length(Biografia) &gt; 50">
                <p style="color:gray; font-size:0.8em;">(Długa biografia)</p>
            </xsl:if>

            <p>Kategoria wagowa: 
                <xsl:choose>
                    <xsl:when test="ParametryFizyczne/Waga &gt; 90">
                        <span style="color:red; font-weight:bold">Waga Ciężka</span>
                    </xsl:when>
                    <xsl:otherwise>
                        <span style="color:green">Waga Lekka/Średnia</span>
                    </xsl:otherwise>
                </xsl:choose>
            </p>

            <xsl:apply-templates select="Osiagniecia"/>
        </div>
        <hr style="border-style: dashed;"/>
    </xsl:template>

    <xsl:template match="Osiagniecia">
        <ul>
            <li>Ranking: <xsl:value-of select="RankingKrajowy"/></li>
            <li>Ostatnie wyniki: <xsl:value-of select="OstatnieCzasy"/> 
                (jednostka: <xsl:value-of select="OstatnieCzasy/@format"/>)
            </li>
            <xsl:if test="Rekordy/RekordDystans">
                <li>
                    <strong>Najlepszy wynik: </strong>
                    <xsl:value-of select="format-number(Rekordy/RekordDystans/Wynik, '#0.00')"/>
                    w <xsl:value-of select="Rekordy/RekordDystans/Miejsce"/>
                </li>
            </xsl:if>
        </ul>
    </xsl:template>

    <xsl:template match="Multimedia">
        <div class="media">
            <xsl:if test="ZdjecieProfilowe/@url">
                <img src="{ZdjecieProfilowe/@url}" alt="Profilowe" class="profilowe"/>
            </xsl:if>
        </div>
    </xsl:template>

    <xsl:template match="@status">
        <xsl:if test="contains(., 'aktywny')">
            <span class="status-aktywny">[AKTYWNY]</span>
        </xsl:if>
        <xsl:if test="contains(., 'nieaktywny')">
            <span class="status-nieaktywny">[NIEAKTYWNY]</span>
        </xsl:if>
    </xsl:template>

    <xsl:template name="stopka">
        <div class="stopka">
            <xsl:copy-of select="$stopkaContent"/>
        </div>
    </xsl:template>

</xsl:stylesheet>